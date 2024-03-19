###############################################################
#
# Makefile for to build library for use with CAP (cdmstools)
# 20120615 -A.Villano this file gets used INSTEAD of "Makefile"
# 	    this is different than one directory up where the
# 	    makeCAP.mk file gets used IN ADDITION TO.
###############################################################

# Suppress the "enterling/leaving" directory messages unconditionally

MAKEFLAGS += --no-print-directory

# Show all commands being executed unless the user sets QUIET

ifdef QUIET
MAKEFLAGS += -s
endif

# Define exit-on-error condition for recursive Make

ifneq (,$(findstring k,$(MAKEFLAGS)))
  EXITK := true
else
  EXITK := exit
endif

#if making locally
ifndef PKG
PKG := linalgebra
endif

#define a package directory at top level
CDMS_PKGDIR := $(PKG)

BUILDDIR := ../

# Define build-target directories locally
INCDIR := $(BUILDDIR)INCLUDE/$(CDMS_PKGDIR)
OBJDIR := $(BUILDDIR)OBJ/$(CDMS_PKGDIR)
LIBDIR := $(BUILDDIR)LIB/$(CDMS_PKGDIR)
BINDIR := $(BUILDDIR)BIN/$(CDMS_PKGDIR)

# ---- Copied from cdmsbats/makefiles/arch.mk

# What flavor of Unix -- Only support Linux and Darwin right now
UNIXTYPE := $(shell uname)

ifeq (,$(findstring Linux,$(UNIXTYPE))$(findstring Darwin,$(UNIXTYPE)))
  $(error $(UNIXTYPE) not supported by CDMSBATS.  Use Linux or MacOSX.)
endif

# Compilation and linking flags
CXXFLAGS += -g -Wall -O4 -fPIC
LDFLAGS += -W

# For SnowLeopard (MacOSX 10.6), need to force 32-bit building
ifeq (Darwin,$(UNIXTYPE))
  DARWIN_VERSION := $(shell uname -r|cut -f1 -d.)
  ifeq (10,$(DARWIN_VERSION))
    CXXFLAGS += -m32 -arch i386
  endif
endif

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
  ARCHLIB := /lib/ /usr/lib/
else ifeq (64,$(ARCH))
  ARCHLIB := /lib64/ /usr/lib64/ /lib/ /usr/lib/
endif

#get everything to do with libblas for your arch
BLASLIB := $(shell find $(ARCHLIB) -nowarn -maxdepth 2 -name libblas* -type f 2> /dev/null)

#if no libblas at all, fail --> give error and stop
#in future flag to just cut of NSOF (or all blas-depend algo) [ANV]
ifeq (,$(BLASLIB))
  $(error $(ARCH)-bit libblas not available on system)
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


# ---- Copied from cdmsbats/makefiles/packages.mk

# Look for directories containing a Makefile
FIND_MAKEFILES := $(wildcard */Makefile)
ifeq (,$(FIND_MAKEFILES))
  $(error Cannot find any CDMSBATS packages or sub-packages)
endif

# Get directory names
PACKAGES := $(FIND_MAKEFILES:/Makefile=)
# ---- End copied from cdmsbats/makefiles/packages.mk

LIBARFILES := $(filter-out ../LIB/linalgebra/libboost-numeric-bindings.a,$(addsuffix .a, $(addprefix ../LIB/linalgebra/lib,$(shell echo $(PACKAGES) |tr A-Z a-z))))

# Header files to be linked for #include resolution
INCFILES := $(wildcard *.h)
ifdef PACKAGES 
INCFILES += $(wildcard $(addsuffix /*.h,$(PACKAGES)))
INCFILES += $(wildcard $(addsuffix /Include/*.h,$(PACKAGES)))
endif
#need path here, how do it if not hard-coded full absolute path
INCFILES := $(addprefix $(shell pwd)/,$(INCFILES))
#get names of inc copies (links)
INCCOPYNAMES := $(addprefix $(INCDIR)/,$(notdir $(INCFILES)))
#full path of static library
LIBRARYFILE := $(LIBDIR)/lib$(PKG).a
SUBLIBRARYFILES := $(LIBARFILES)

#need phony tag for $PACKAGES since nothing will  result from it

.PHONY : all sub.ccode sub.purge sub.install inc lib bin clean veryclean $PACKAGES

all: lib 

sub.purge sub.ccode sub.install:
	@for pkg in $(PACKAGES) ; do \
	  $(MAKE) $${pkg}.$@ -f makeCAP.mk || $(EXITK) ;\
	 done


clean: sub.purge
	@echo Making clean...
	@echo for subpackage $(PKG)

#	$(RM) -rf $(BUILDDIR) ;\
#remove everything with veryclean tag
veryclean : sub.purge
	$(RM) -rf BUILD ;\
	find . -name '*~' -exec $(RM) -rf \{\} \;

inc: $(INCCOPYNAMES) boost-numeric-bindings.sub.inc
	@echo inc made...
	@echo for subpackage $(PKG)

lib:  $(LIBRARYFILE) 
	@echo lib made...
	@echo for subpackage $(PKG)
bin:
	@echo No bin to make...
	@echo for subpackage $(PKG)

sub.install: sub.ccode

#actually everything only depends on the static library
$(LIBRARYFILE): $(INCCOPYNAMES) $(SUBLIBRARYFILES) 
	@echo Making lib...
	@echo for subpackage $(PKG)
	@echo $(LIBRARYFILE) 
	@echo $(SUBLIBRARYFILES)
	#@ls -lh '$(LIBDIR)/lib$(PKG).a'
	@for arch in $(addprefix ../,$(LIBARFILES)) ; do \
	  ( cd $(OBJDIR) ; $(AR) -x $${arch} || $(EXITK) ; )\
	 done
	  ( cd $(LIBDIR) ; $(AR) rv lib$(PKG).a ../../OBJ/linalgebra/*.o ;)

$(INCCOPYNAMES):
	@echo Making inc...
	@echo for subpackage $(PKG)
ifdef INCFILES
	@$(LN) -fs $(INCFILES) $(INCDIR)
endif

$(SUBLIBRARYFILES):
	@echo Making sub.install...
	#@for file in $(SUBLIBRARYFILES) ; do \
	#  ( ls -lh $${file} ;) \
	# done
	$(MAKE) -f makeCAP.mk sub.install

#be careful with these recursive dependencies, make sure only get executed
#once for each subpackage not Npkg times for each subpackage
#$(PACKAGES:=.sub.install) : sub.ccode 

$(PACKAGES:=.sub.purge) \
$(PACKAGES:=.sub.ccode) \
$(PACKAGES:=.sub.install): 
	@pkg=$(basename $(basename $@)) ; tgt=$(subst .,,$(suffix $@)) ;\
	 echo "Building $$tgt in $$pkg ..." ;\
	 $(MAKE) -C $$pkg TOPLEVELPKG=$(PKG) PKG=$$pkg CDMS_CAP_MAKE=$(CDMS_CAP_MAKE) $$tgt || $(EXITK)

boost-numeric-bindings.sub.inc: 
	@pkg=$(basename $(basename $@)) ; tgt=$(subst .,,$(suffix $@)) ;\
	 echo "Building $$tgt in $$pkg ..." ;\
	 $(MAKE) -C $$pkg TOPLEVELPKG=$(PKG) PKG=$$pkg CDMS_CAP_MAKE=$(CDMS_CAP_MAKE) $$tgt || $(EXITK)

SUBMAKECMDGOALS := $(subst .,,$(suffix $(MAKECMDGOALS)))
ifneq (veryclean,$(MAKECMDGOALS))
ifneq (purge,$(SUBMAKECMDGOALS))
# ---- Copied from cdmsbats/makefiles/setup.mk
DUMMY := $(shell $(MKDIR) -p BUILD/include BUILD/obj BUILD/lib BUILD/bin)

#allow definition of package directory as well
ifdef CDMS_PKGDIR
DUMMY1 := $(shell $(MKDIR) -p ../INCLUDE/$(CDMS_PKGDIR) ../BIN/$(CDMS_PKGDIR) ../LIB/$(CDMS_PKGDIR) ../OBJ/$(CDMS_PKGDIR))
endif
endif
endif
