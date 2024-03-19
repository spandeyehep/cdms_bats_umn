###############################################################
#  Makefile for linalgebra library 
#
#
#
# 20120405  A. Villano -- Subpackage for linear algebra and sparce
#                         matrices.
###############################################################

MAKEFLAGS += --no-print-directory

#for including standard makefiles
CDMS_MAKEFILES := ../../makefiles/

# Do platform-specific configuration
# how do I make sure this isn't redundantly included?
include $(CDMS_MAKEFILES)arch.mk

#if making locally
ifndef PKG
PKG := linalgebra
endif

#define a package directory at top level
CDMS_PKGDIR := $(PKG)

# Do this for subdirectories 
-include $(CDMS_MAKEFILES)packages.mk

# Define exit-on-error condition for recursive Make

ifneq (,$(findstring k,$(MAKEFLAGS)))
  EXITK := true
else
  EXITK := exit
endif

BUILDDIR := $(CDMS_MAKEFILES)../BUILD/$(PKG)/

# Define build-target directories locally
INCDIR := $(BUILDDIR)include
OBJDIR := $(BUILDDIR)obj
LIBDIR := $(BUILDDIR)lib
BINDIR := $(BUILDDIR)bin

LIBARFILES := $(filter-out ../lib/libboost-numeric-bindings.a,$(addsuffix .a, $(addprefix ../lib/lib,$(shell echo $(PACKAGES) |tr A-Z a-z))))

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
SUBLIBRARYFILES := $(addprefix $(LIBDIR)/,$(LIBARFILES))


#need phony tag for $PACKAGES since nothing will  result from it

.PHONY : all sub.ccode sub.purge sub.install inc lib bin clean veryclean $PACKAGES

all: lib 

sub.purge sub.ccode sub.install:
	@for pkg in $(PACKAGES) ; do \
	  $(MAKE) $${pkg}.$@ || $(EXITK) ;\
	 done


clean: sub.purge
	@echo Making clean...
	@echo for subpackage $(PKG)

#remove everything with veryclean tag
veryclean : sub.purge
	$(RM) -rf $(BUILDDIR) ;\
	$(RM) -rf BUILD ;\
	find . -name '*~' -exec $(RM) -rf \{\} \;

inc: $(INCCOPYNAMES) boost-numeric-bindings.sub.inc
	@echo inc made...
	@echo for subpackage $(PKG)
	#@ls -lh $(LIBRARYFILE)

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
	@echo 'file $(LIBDIR)/lib$(PKG).a'
	#@ls -lh '$(LIBDIR)/lib$(PKG).a'
	@for arch in $(LIBARFILES) ; do \
	  ( cd $(OBJDIR) ; $(AR) -x $${arch} || $(EXITK) ; )\
	 done
	  ( cd $(LIBDIR) ; $(AR) rv lib$(PKG).a ../obj/*.o ;)

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
	$(MAKE) sub.install

#be careful with these recursive dependencies, make sure only get executed
#once for each subpackage not Npkg times for each subpackage
#$(PACKAGES:=.sub.install) : sub.ccode 

$(PACKAGES:=.sub.purge) \
$(PACKAGES:=.sub.ccode) \
$(PACKAGES:=.sub.install): 
	@pkg=$(basename $(basename $@)) ; tgt=$(subst .,,$(suffix $@)) ;\
	 echo "Building $$tgt in $$pkg ..." ;\
	 $(MAKE) -C $$pkg TOPLEVELPKG=$(PKG) PKG=$$pkg $$tgt || $(EXITK)

boost-numeric-bindings.sub.inc: 
	@pkg=$(basename $(basename $@)) ; tgt=$(subst .,,$(suffix $@)) ;\
	 echo "Building $$tgt in $$pkg ..." ;\
	 $(MAKE) -C $$pkg TOPLEVELPKG=$(PKG) PKG=$$pkg $$tgt || $(EXITK)

SUBMAKECMDGOALS := $(subst .,,$(suffix $(MAKECMDGOALS)))
ifneq (veryclean,$(MAKECMDGOALS))
ifneq (purge,$(SUBMAKECMDGOALS))
include $(CDMS_MAKEFILES)setup.mk
endif
endif

# If building under CDMSTOOLS, use alternate configuration
ifndef PKG
-include makeCAP.mk
endif
