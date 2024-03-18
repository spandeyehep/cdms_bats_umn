###############################################################
# $Id$
#
# Makefile for BatRoot
###############################################################

#trick for getting the git version in the code
CDMSBATS_GIT_VERSION = $(shell sh -c 'git describe --abbrev=100 --always')
BATCOMM_GIT_VERSION = $(shell sh -c 'cd ../BatCommon; git describe --abbrev=100 --always')

CXXFLAGS += -D__CB_GIT_VERSION=\"$(CDMSBATS_GIT_VERSION)\"
CXXFLAGS += -D__BC_GIT_VERSION=\"$(BATCOMM_GIT_VERSION)\"

# Executables to be built (must have matching .cxx files)
BINS := BatNoise

# Library to be built
LIBNAME := BatNoise

# Set up CDMS build system
include ../makefiles/cdmsbats.mk

