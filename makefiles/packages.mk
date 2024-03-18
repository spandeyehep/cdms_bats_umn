###############################################################
# $Id$
# Generate list of recognizable CDMSBATS packages
#
# 20120414  A. Villano -- use CDMS_MAKEFILES variable for including
#                         other makes from this directory when this
#                         makefile is itself included from a directory
#                         which is not exactly one directory below the
#                         top cdmsbats directory (i.e. ../makefiles/
#                         is *not* the correct path).
###############################################################

#allow includes from more than one directory below if CDMS_MAKEFILES specified
ifndef CDMS_MAKEFILES
CDMS_MAKEFILES := ../makefiles/
endif

# Look for directories containing a Makefile
FIND_MAKEFILES := $(wildcard */Makefile)
ifeq (,$(FIND_MAKEFILES))
  $(error Cannot find any CDMSBATS packages or sub-packages)
endif

# Get directory names
PACKAGES := $(FIND_MAKEFILES:/Makefile=)
