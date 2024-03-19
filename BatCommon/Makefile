###############################################################
# $Id$
#
# Makefile for BatCommon library
#
# 20120405  A. Villano -- Support making of sub-packages 
###############################################################

# Do this for subpackages 
-include ../makefiles/packages.mk

#diagnostic binary file [ANV]
#comment in if want to use (NOTE need simpleNSOF.cxx)
#BINS := simpleNSOF
SKIPFILES := simpleNSOF.cxx dbtestmain.cxx

# Library to be built
LIBNAME := Cdmsbats

# Subdirectories containing library files
DIRS := analysis datareader extdata management pulse BatMath utilities \
	extdata/database

# set dependency for inc,lib,bin,clean targets wrt sub packages
CDMS_SUB_INC := sub.inc
CDMS_SUB_LIB := sub.all
CDMS_SUB_BIN :=
CDMS_SUB_CLEAN := sub.clean

# Set up CDMSBATS build system
-include ../makefiles/cdmsbats.mk

# Make the sub-packages with dependency specified above
-include ../makefiles/makesubpkg.mk

# If building under CDMSTOOLS, use alternate configuration
ifndef PKG
-include makeCAP.mk
endif
