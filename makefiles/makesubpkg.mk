###############################################################
# makefile to build subpackages within any directory subpackages are
# directories with makefiles and specific targets with code in
# subdirectories which will be included in the top-level package
# being built
#
# 20120405 - A. Villano initial revision of a makefile that allows
#            the provision of pre-packaged (with Makefile) subpackages.
# 20120414  A. Villano -- use CDMS_MAKEFILES variable for including
#                         other makes from this directory when this
#                         makefile is itself included from a directory
#                         which is not exactly one directory below the
#                         top cdmsbats directory (i.e. ../makefiles/
#                         is *not* the correct path).
#
###############################################################

#allow includes from more than one directory below if CDMS_MAKEFILES specified
ifndef CDMS_MAKEFILES
CDMS_MAKEFILES := ../makefiles/
endif

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

.PHONY : sub.all sub.inc sub.lib sub.bin sub.clean sub.veryclean $PACKAGES

sub.all : sub.bin

sub.clean sub.veryclean sub.inc sub.lib sub.bin :
	@for pkg in $(PACKAGES) ; do \
	  $(MAKE) $${pkg}.$@ || $(EXITK) ;\
	 done

#make the veryclean tag just get passed on to the next directory
#sub.veryclean :
#	$(RM) -rf BUILD ;\
#	find . -name '*~' -exec $(RM) -rf \{\} \;
# Global dependencies

sub.bin : sub.lib 

sub.lib : sub.inc

#be careful with these recursive dependencies, make sure only get executed
#once for each subpackage not Npkg times for each subpackage
#$(PACKAGES:=.sub.lib) : sub.inc  


# Subdirectory targets


$(PACKAGES:=.sub.veryclean) \
$(PACKAGES:=.sub.clean) \
$(PACKAGES:=.sub.inc) \
$(PACKAGES:=.sub.lib) \
$(PACKAGES:=.sub.bin) :
	 @pkg=$(basename $(basename $@)) ; tgt=$(subst .,,$(suffix $@)) ;\
	 [ $$tgt != inc ] && echo "Building $$tgt in $$pkg ..." ;\
	 $(MAKE) -C $$pkg PKG=$$pkg $$tgt || $(EXITK)
