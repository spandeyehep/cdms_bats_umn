###############################################################
# $Id$
# Directory configuration Makefile for CDMSBATS
#
# Usage:  include makefiles/setup.mk (at end of top-level Makefile)
#
# Creates LIB, OBJ and INC directories for use during builds
#
#-----
# 20110906  M. Kelsey
# 20120414  A. Villano -- use CDMS_MAKEFILES variable for including
#                         other makes from this directory when this
#                         makefile is itself included from a directory
#                         which is not exactly one directory below the
#                         top cdmsbats directory (i.e. ../makefiles/
#                         is *not* the correct path).
# 20120416  A. Villano -- add optional CDMS_PKGDIR definition to build
#                         subpackage directories under BUILD/ at top level
# 20120513  A. Villano -- add some statements in arch.mk to make attempt
#                         to link libblas even if .a or .so doesn't exist
###############################################################

#allow includes from more than one directory below if CDMS_MAKEFILES specified
ifndef CDMS_MAKEFILES
CDMS_MAKEFILES := ../makefiles/
endif

include $(CDMS_MAKEFILES)arch.mk

DUMMY := $(shell $(MKDIR) -p BUILD/include BUILD/obj BUILD/lib BUILD/bin)

#allow definition of package directory as well
ifdef CDMS_PKGDIR
DUMMY1 := $(shell $(MKDIR) -p $(CDMS_MAKEFILES)../BUILD/$(CDMS_PKGDIR)/include $(CDMS_MAKEFILES)../BUILD/$(CDMS_PKGDIR)/obj $(CDMS_MAKEFILES)../BUILD/$(CDMS_PKGDIR)/lib $(CDMS_MAKEFILES)../BUILD/$(CDMS_PKGDIR)/bin)
endif

#if BLAS was found by arch.mk then put a simlink for libblas.so in BUILD/lib/
ifdef BLASLINKDIR
DUMMY2 := $(shell $(LN) -fs $(BLASLINKLIB)  BUILD/lib/libblas.so)
endif
