###############################################################
# $Id$
# Top-level Makefile for CDMSBATS
#
# Users can build BatRoot and BatCalib directly from here.
#
# Users can also invoke specific targets in BatRoot or BatCalib
# with |make XXX.target|.
#
# 20111201  M. Kelsey -- Support termination of recursive makes when any
#	package has an error; with termination suppressed by "-k" flag
###############################################################

# Define list of known packages to be built

#for including standard makefiles
CDMS_MAKEFILES := makefiles/

include $(CDMS_MAKEFILES)packages.mk

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

# Build targets -- default builds everything

.PHONY : all inc lib bin clean veryclean $PACKAGES

all : bin

clean inc lib bin :
	@for pkg in $(PACKAGES) ; do \
	  echo "Building $$pkg";\
	  $(MAKE) $${pkg}.$@ || $(EXITK) ;\
	 done

veryclean :
	$(RM) -rf BUILD ;\
	find . -name '*~' -exec $(RM) -rf \{\} \;
	$(MAKE) -C BatCommon/linalgebra veryclean || $(EXITK)

# Global dependencies

bin : lib

$(PACKAGES:=.lib) \
$(PACKAGES:=.bin) : inc

# Subdirectory targets

$(PACKAGES) : %:%.bin

$(PACKAGES:=.clean) \
$(PACKAGES:=.inc) \
$(PACKAGES:=.lib) \
$(PACKAGES:=.bin) :
	@pkg=$(basename $@) ; tgt=$(subst .,,$(suffix $@)) ;\
	 [ $$tgt != inc ] && echo "Building $$tgt in $$pkg ..." ;\
	 $(MAKE) -C $$pkg PKG=$$pkg $$tgt || $(EXITK)

# "Utility" to create directories and definitions before targets

ifneq (veryclean,$(MAKECMDGOALS))
include $(CDMS_MAKEFILES)setup.mk
endif
