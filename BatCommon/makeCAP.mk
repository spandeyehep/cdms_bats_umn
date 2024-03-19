###############################################################
# $Id$
#
# Makefile for to build library for use with CAP (cdmstools)
# 20120614  A. Villano -- Modified to define exit-on-error for
# 			  sub-package make loops and to have veryclean tag
###############################################################

# define for switches in lower directories with this CAP making
CDMS_CAP_MAKE:= true

# Suppress the "enterling/leaving" directory messages unconditionally

MAKEFLAGS += --no-print-directory

# Show all commands being executed unless the user sets QUIET

ifdef QUIET
MAKEFLAGS += -s
endif

# When building from within package, make sure all includes are set up
# Also maybe shouldn't do this if making clean/veryclean sub.clean/sub.veryclean? [ANV]
ifndef PKG
PKG := $(shell pwd|xargs basename)
endif

# Define exit-on-error condition for recursive Make

ifneq (,$(findstring k,$(MAKEFLAGS)))
  EXITK := true
else
  EXITK := exit
endif

# in some cases this is redundant but in the special case
# of a make call to makeCAP.mk with target inc it is not
# in any case this is the value of DIRS that will be used for CAP
# making. [ANV]
# Subdirectories containing library files
DIRS := analysis datareader extdata management pulse BatMath

# Dummy target to force library build, only.

all : lib

# Destination for dependencies and object files
OBJDIR := OBJ
LIBDIR := LIB
BINDIR := BIN
INCDIR := INCLUDE
DUMMY := $(shell mkdir -p $(OBJDIR))	# Ensure that directory pre-exists
DUMMY := $(shell mkdir -p $(LIBDIR))	# Ensure that directory pre-exists
DUMMY := $(shell mkdir -p $(BINDIR))	# Ensure that directory pre-exists
DUMMY := $(shell mkdir -p $(INCDIR))	# Ensure that directory pre-exists

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
  export C_INCLUDE_PATH := $(C_INCLUDE_PATH):/sw/include
  export CPLUS_INCLUDE_PATH := $(CPLUS_INCLUDE_PATH):/sw/include
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

# ---- Copied from cdmsbats/makefiles/setup.mk (with modification)

ifdef BLASLINKDIR
DUMMY2 := $(shell $(LN) -fs $(BLASLINKLIB)  $(LIBDIR)/libblas.so)
endif

# ---- Copied from cdmsbats/makefiles/cdmsbats.mk

LIBCCFILES := $(wildcard *.cxx)
ifdef DIRS
LIBCCFILES += $(wildcard $(addsuffix /*.cxx,$(DIRS)))
endif

LIBCCFILES := $(filter-out $(SKIPFILES),$(LIBCCFILES))
LIBOFILES := $(addprefix $(OBJDIR)/,$(notdir $(LIBCCFILES:.cxx=.o)))

# Header files to be linked for #include resolution
INCFILES := $(wildcard *.h)
ifdef DIRS
INCFILES += $(wildcard $(addsuffix /*.h,$(DIRS)))
endif

# get the names of incfiles in $(INCDIR)
INC := $(addprefix $(INCDIR)/,$(notdir $(INCFILES)))

# Static library to be built (not supporting shared libraries)
LIB := $(LIBDIR)/lib$(strip $(LIBNAME)).a

# Dependency files for subsequent builds
DEPFILES := $(LIBOFILES:.o=.d)

# Specify search path for headers and source files
CPPFLAGS += -I. -I$(INCDIR) -I$(INCDIR)/linalgebra/ -I$(INCDIR)/linalgebra/boost-numeric/include/

ifdef DIRS
CPPFLAGS += $(addprefix -I,$(DIRS))
VPATH := $(subst  ,:,$(DIRS))
endif


# Add references to ROOT at compile and link time
ifdef ROOTSYS
CPPFLAGS += -I$(shell $(ROOTSYS)/bin/root-config --incdir)
CXXFLAGS += $(shell $(ROOTSYS)/bin/root-config --cflags) 
else
DUMMY := $(shell root-config --incdir)
ifeq (,$(DUMMY))
$(error CDMSBATS requires ROOT. Please define the ROOTSYS envvar.)
endif
CPPFLAGS += -I$(shell root-config --incdir)
CXXFLAGS += $(shell root-config --cflags) 
endif

# include MIDAS if available
ifdef MIDASTOOLSSYS
LDFLAGS += $(MIDASTOOLSSYS)/common/midasControl/libMidasControl.a
CXXFLAGS += -I$(MIDASTOOLSSYS)/common/midasControl 
CPPFLAGS += -I$(MIDASTOOLSSYS)/common/midasControl 
endif

ifdef MIDASSYS

LDFLAGS +=  $(MIDASSYS)/linux/lib/libmidas.a -lutil -lrt
CXXFLAGS += -DHAVE_MIDAS -I$(MIDASSYS)/include 
CPPFLAGS += -DHAVE_MIDAS -I$(MIDASSYS)/include 

# include MIDASTOOLS if available
ifdef MIDASTOOLSSYS
LDFLAGS += $(MIDASTOOLSSYS)/common/midasControl/libMidasControl.a
CXXFLAGS += -I$(MIDASTOOLSSYS)/common/midasControl 
CPPFLAGS += -I$(MIDASTOOLSSYS)/common/midasControl 
else
LDFLAGS +=  $(MIDASSYS)/../../online/midasControl/libMidasControl.a 
CXXFLAGS += -I$(MIDASSYS)/../../online/midasControl
CPPFLAGS += -I$(MIDASSYS)/../../online/midasControl
endif

endif



# Don't pull in (or trigger building) dependency files when deleting
# can't be sub.inc either but this is tricky because never gets called as pure sub.inc!! [ANV]
SUBMAKECMDGOALS := $(subst .,,$(suffix $(MAKECMDGOALS)))
ifneq (inc,$(MAKECMDGOALS))
ifneq (inc,$(SUBMAKECMDGOALS))
ifneq (clean,$(MAKECMDGOALS))
ifneq (veryclean,$(MAKECMDGOALS))
-include $(DEPFILES)
endif
endif
endif
endif

# Define targets for building
#
inc : $(CDMS_SUB_INC)
ifdef INCFILES
	@$(LN) -fs $(addprefix ../,$(INCFILES)) $(INCDIR)
endif

#need to add something to stop the CDMS_SUB_LIB dependency
#from triggering another build of subdirectory
lib : $(CDMS_SUB_LIB) $(LIB)

clean : $(CDMS_SUB_CLEAN)
	@/bin/rm -f $(LIB) $(BINS) $(LIBOFILES) $(BINOFILES) \
	 $(DEPFILES) *~ $(addsuffix /*~,$(DIRS))

veryclean :
	$(RM) -rf INCLUDE ;\
	$(RM) -rf LIB ;\
	$(RM) -rf BIN ;\
	$(RM) -rf OBJ ;\
	$(MAKE) -C linalgebra -f makeCAP.mk veryclean || $(EXITK)

.PHONY : lib clean

# Define dependencies

$(LIB) : $(LIBOFILES)
	@echo "Building $@ library ..."
	@$(AR) -rc $@ $^ && $(RANLIB) $@

$(LIBOFILES) : $(OBJDIR)/%.o:%.cxx 
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $^ -o $@

$(DEPFILES) : $(OBJDIR)/%.d:%.cxx $(INC)
	@$(CPP) $(CPPFLAGS) -M -MF $@ -c $(filter-out $(INC),$^)

$(INC) :
	@$(MAKE) --no-print-directory -s PKG=BatCommon -f makeCAP.mk inc
	@$(MAKE) --no-print-directory -s PKG=BatCommon -f makeCAP.mk sub.inc

# ---- Copied from cdmsbats/makefiles/makesubpkg.mk

# Build targets -- default builds everything

.PHONY : sub.all sub.inc sub.lib sub.bin sub.clean sub.veryclean $PACKAGES

sub.all : sub.bin

sub.clean sub.veryclean sub.inc sub.lib sub.bin :
	@for pkg in $(PACKAGES) ; do \
	  $(MAKE) -f makeCAP.mk $${pkg}.$@ || $(EXITK) ;\
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
	 $(MAKE) -C $$pkg -f makeCAP.mk PKG=$$pkg TOPPKG=$(PKG) CDMS_CAP_MAKE=$(CDMS_CAP_MAKE) $$tgt || $(EXITK)
