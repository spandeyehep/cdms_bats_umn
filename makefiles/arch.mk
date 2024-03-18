###############################################################
# $Id$
# Architecture-dependendent Makefile for CDMSBATS packages
#
# 20110324  M. Kelsey -- Add support to find and strip main() from library
# 20110325  M. Kelsey -- Need to capture MacOSX version directly (not preset)
# 20120414  A. Villano -- use CDMS_MAKEFILES variable for including
#                         other makes from this directory when this
#                         makefile is itself included from a directory
#                         which is not exactly one directory below the
#                         top cdmsbats directory (i.e. ../makefiles/
#                         is *not* the correct path).
# 20120513  A. Villano -- Modified to try to find libblas if exist
#                         and link even if not in /usr/lib/ or /usr/lib64/
###############################################################

#allow includes from more than one directory below if CDMS_MAKEFILES specified
ifndef CDMS_MAKEFILES
CDMS_MAKEFILES := ../makefiles/
endif

# What flavor of Unix -- Only support Linux and Darwin right now
UNIXTYPE := $(shell uname)

ifeq (,$(findstring Linux,$(UNIXTYPE))$(findstring Darwin,$(UNIXTYPE)))
  $(error $(UNIXTYPE) not supported by CDMSBATS.  Use Linux or MacOSX.)
endif

# Compilation and linking flags
CXXFLAGS += -g -Wall -O4
LDFLAGS += -W

# Commands which may not be defined on all platforms
override LN     := /bin/ln
override MKDIR  := /bin/mkdir
override RANLIB := /usr/bin/ranlib

TESTLINUX := $(findstring Linux,$(UNIXTYPE))
ifdef TESTLINUX
# Get architecture type 32 or 64 for 32-bit or 64-bit
ARCH := $(shell getconf LONG_BIT)
#check for libblas on the system
ifeq (32,$(ARCH))
  # can we explicitly check the architecture of the .so?
  ARCHLIB := /lib/ /usr/lib/
else ifeq (64,$(ARCH)) 
# can we explicitly check the architecture of the .so?
  ARCHLIB := /lib64/ /usr/lib64/ /lib/ /usr/lib/
endif

#get everything to do with libblas for your arch
#ARCHLIBBLAS := $(addsuffix libblas, $(ARCHLIB))
BLASLIB := $(shell find $(ARCHLIB) -nowarn -maxdepth 2 -name libblas* -type f 2> /dev/null)

#if no libblas at all, fail --> give error and stop
#in future flag to just cut of NSOF (or all blas-depend algo) [ANV]
ifeq (,$(BLASLIB))
  #$(error $(ARCH)-bit libblas not available on system)
endif

#get only the linkable libraries
BLASLINKLIB := $(filter %.a %.so,$(BLASLIB))

#get the dirs that have directly linkable stuff
#BLASLINKDIR := $(sort $(dir $(BLASLINKLIB)))

#if this has more than one linkable dir choose to do nothing
#(since ld probably links to these directories automatically)
#or link explicitly to first one
ifeq (,$(BLASLINKLIB))
  #if no linkable stuff go back to $(BLASLIB) and find candidate linkable
  #list all shared libs that ARENT simlinks and take the first one (i guess) [ANV]
  BLASLINKLIB := $(firstword $(shell find $(ARCHLIB) -nowarn -maxdepth 2 -name libblas* -type f 2> /dev/null |grep .so))
  BLASLINKDIR := $(dir $(BLASLINKLIB))
endif

#if BLASLINKDIR is empty at this point, don't do anything
endif


# Support symbols to eliminate accidental "main()" from libraries
override NM := /usr/bin/nm -AP

ifeq (Darwin,$(UNIXTYPE))
  MAINSYMB := " _main T"
  export C_INCLUDE_PATH := $(C_INCLUDE_PATH):/sw/include
  export CPLUS_INCLUDE_PATH := $(CPLUS_INCLUDE_PATH):/sw/include
else
  MAINSYMB := " main T"
  NM += -C
endif
