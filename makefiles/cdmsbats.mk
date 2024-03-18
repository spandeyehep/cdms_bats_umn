###############################################################
# $Id$
# Controlling Makefile for CDMSBATS packages
#
# Usage:  include ../makefiles/cdmsbats.mk (at end of Makefile)
#
# User must define the following macros in the package Makefile:
# BINS		:= List of executables to be built
# LIBNAME	:= Base name of library (e.g., "XYZ" for libXYZ.a)
#
# If LIBNAME is not defined, the name of the directory is used
#
# User may define the following optional macros:
# DIRS		:= Subdirectories containing source files
# SKIPFILES     := Source files to be kept out of library
# BINCCFILES	:= Additional files to be compiled for executable(s)
#
#-----
# 20110324  M. Kelsey -- Protect against some undefined variables, strip
#		occurrences of "main()" from library
# 20110325  M. Kelsey -- Change order of arguments for linking executable
# 20110906  M. Kelsey -- Add top-level INC, OBJ, and LIB directories, use of
#		BatCommon library
# 20111017  M. Kelsey -- Add BIN directory, allow LIBNAME to be undefined
# 20111024  M. Kelsey -- Put build directories under "BUILD"
# 20111201  M. kelsey -- Add "all" target to build package w/o arguments.
#		Add "magic action" done via $(shell) to build global includes
# 20120315  M. Kelsey -- Add "-MT" option for dependency generation, and change
#		"$^" to "$<" for both .d and .o compiler actions.
# 20120414  A. Villano -- use CDMS_MAKEFILES variable for including
#                         other makes from this directory when this
#                         makefile is itself included from a directory
#                         which is not exactly one directory below the
#                         top cdmsbats directory (i.e. ../makefiles/
#                         is *not* the correct path).
# 20120418  A. Villano -- use CDMS_SUB_* variables
#                         for including subpackage dependencies correctly
#                         * = {INC,LIB,BIN,CLEAN} this might give too
#                         much freedom to the user, if abused.
# 20150603 J. Morales -- Added option to compile without issues
#			 on Brazos (A&M) using miniconda ROOT 
#			 look for if $HOSTNAME = "brazos.tamu.edu"
###############################################################

ifndef CDMS_SUB_INC
CDMS_SUB_INC :=
endif
ifndef CDMS_SUB_LIB
CDMS_SUB_LIB :=
endif
ifndef CDMS_SUB_BIN
CDMS_SUB_BIN :=
endif
ifndef CDMS_SUB_CLEAN
CDMS_SUB_CLEAN :=
endif
CDMS_SUB_INC :=$(strip $(CDMS_SUB_INC))
CDMS_SUB_LIB :=$(strip $(CDMS_SUB_LIB))
CDMS_SUB_BIN :=$(strip $(CDMS_SUB_BIN))
CDMS_SUB_CLEAN :=$(strip $(CDMS_SUB_CLEAN))

#allow includes from more than one directory below if CDMS_MAKEFILES specified
ifndef CDMS_MAKEFILES
CDMS_MAKEFILES := ../makefiles/
endif

# Show all commands being executed unless the user sets QUIET
ifdef QUIET
MAKEFLAGS += -s
endif

# When building from within package, make sure all includes are set up
# Also maybe shouldn't do this if making clean/veryclean sub.clean/sub.veryclean? [ANV]
ifndef PKG
PKG := $(shell pwd|xargs basename)
TOP_INC := $(shell $(MAKE) --no-print-directory -s -C.. inc)
endif

# Define targets, including default to build everything w/o args.
.PHONY : all inc lib bin clean

all : bin

# Do platform-specific configuration
include $(CDMS_MAKEFILES)arch.mk

# Define build-target directories
INCDIR := ../BUILD/include
OBJDIR := ../BUILD/obj
LIBDIR := ../BUILD/lib
BINDIR := ../BUILD/bin

# Get lists of source files for building
ifdef BINS
BINS       := $(strip $(BINS))
BINFILES   := $(addprefix $(BINDIR)/,$(BINS))
BINCCFILES += $(BINS:=.cxx)
endif

LIBCCFILES := $(wildcard *.cxx)
ifdef DIRS
LIBCCFILES += $(wildcard $(addsuffix /*.cxx,$(DIRS)))
endif

LIBCCFILES := $(filter-out $(SKIPFILES) $(BINCCFILES),$(LIBCCFILES))
LIBOFILES  := $(addprefix $(OBJDIR)/,$(notdir $(LIBCCFILES:.cxx=.o)))
BINOFILES  := $(addprefix $(OBJDIR)/,$(notdir $(BINCCFILES:.cxx=.o)))

# Header files to be linked for #include resolution
INCFILES := $(wildcard *.h)
ifdef DIRS
INCFILES += $(wildcard $(addsuffix /*.h,$(DIRS)))
endif

# Static library to be built (not supporting shared libraries)
ifndef LIBNAME
LIBNAME := $(PKG)
endif
LIB := $(LIBDIR)/lib$(strip $(LIBNAME)).a

#***********FIX ME?*******************
# Dependency files for subsequent builds
# these depfiles exist even if the .o files don't, that's awkward [ANV]
# http://scottmcpeak.com/autodepend/autodepend.html 
DEPFILES := $(LIBOFILES:.o=.d) $(BINOFILES:.o=.d)

# Specify search path for headers and source files
ifneq (,$(findstring brazos.tamu,$(shell hostname)))
CPPFLAGS += -I. -I$(INCDIR) -I$(INCDIR)/../linalgebra/include/ -I$(INCDIR)/../linalgebra/boost-numeric/include/ -I/home/hepxadmin
else
CPPFLAGS += -I. -I$(INCDIR) -I$(INCDIR)/../linalgebra/include/ -I$(INCDIR)/../linalgebra/boost-numeric/include/
endif

ifdef DIRS
### CPPFLAGS += $(addprefix -I,$(DIRS))
VPATH := $(subst  ,:,$(DIRS))
endif

# Specify search path for CDMSBATS libraries
# also specifiy explicit dependence on linalgebra library
LDFLAGS += -L$(LIBDIR) -l$(LIBNAME) -lCdmsbats -L$(LIBDIR)/../linalgebra/lib/ -llinalgebra

# Add references to ROOT at compile and link time
ifneq (,$(findstring brazos.tamu,$(shell hostname)))
CPPFLAGS += -I$(shell root-config --incdir)
CXXFLAGS += $(shell root-config --cflags) 
LDFLAGS  += $(shell root-config --libs) -lMinuit -lGui 
else
ifndef ROOTSYS
$(error CDMSBATS requires ROOT.  Please define the ROOTSYS envvar.)
endif
CPPFLAGS += -I$(shell $(ROOTSYS)/bin/root-config --incdir)
CXXFLAGS += $(shell $(ROOTSYS)/bin/root-config --cflags) 
LDFLAGS  += $(shell $(ROOTSYS)/bin/root-config --libs) -lMinuit -lGui 
endif

#explicitly activate necessary c++11 stuff
#use the older but now deprecated version for older compilers
CPPFLAGS += -std=c++0x

# Add the libblas (Basic Linear Algebra System) unconditionally

#explicitly include libblas
LDFLAGS += -lblas

# Additional system libraries
LDFLAGS += -lz

# Special: Build BatCommon library from top level if not found
ifneq (BatCommon,$(PKG))
$(LIBDIR)/libCdmsbats.a :
	@$(MAKE) -C.. BatCommon.lib
$(LIBDIR)/../linalgebra/lib/liblinalgebra.a :
	@$(MAKE) -C.. BatCommon.lib
endif

# Define targets for building

inc : $(CDMS_SUB_INC)
ifdef INCFILES
	@$(LN) -fs $(addprefix ../../$(PKG)/,$(INCFILES)) $(INCDIR)
endif

#need to add something to stop the CDMS_SUB_LIB dependency
#from triggering another build of subdirectory
lib : $(CDMS_SUB_LIB) $(LIB)

bin : lib
bin : $(CDMS_SUB_BIN) $(BINFILES)

clean : $(CDMS_SUB_CLEAN)
	@/bin/rm -f $(LIB) $(BINFILES) $(LIBOFILES) $(BINOFILES) \
	  $(DEPFILES) $(addsuffix /*~,. $(DIRS))

# Define dependencies

$(LIB) : $(LIBOFILES)
	 $(AR) -rc $@ $^ && $(RANLIB) $@
	@$(StripMain)

$(LIBOFILES) $(BINOFILES) : $(OBJDIR)/%.o:%.cxx
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

ifdef BINS
.PHONY : $(BINS)
$(BINS) : %:$(BINDIR)/%

$(BINFILES) : $(BINDIR)/%:$(OBJDIR)/%.o $(LIB) $(LIBDIR)/libCdmsbats.a $(LIBDIR)/../linalgebra/lib/liblinalgebra.a
	@echo "Linking $(notdir $@) ..."
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
endif	## BINS

$(DEPFILES) : $(OBJDIR)/%.d:%.cxx
	@$(CPP) $(CPPFLAGS) -M -MF $@ -MT $(@:.d=.o) -c $<

# Don't pull in (or trigger building) dependency files when deleting
# can't be sub.inc either but this is tricky because never gets called as pure sub.inc!! [ANV]
SUBMAKECMDGOALS := $(subst .,,$(suffix $(MAKECMDGOALS)))
ifneq (clean,$(MAKECMDGOALS))
ifneq (inc,$(MAKECMDGOALS))
ifneq (inc,$(SUBMAKECMDGOALS))
-include $(DEPFILES)
endif
endif
endif

# Special action to remove accidental main() from library
# Output of $(NM) is "library-path/lib.a[mod.o]: ....symbol-data...."
# HASMAIN will contain either library/module portion, or be empty

override define StripMain
  HASMAIN=`$(NM) $(LIB)|grep $(MAINSYMB)|cut -f1 -d:|sort -u` ;\
  if [ "$$HASMAIN" ]; then \
    MOD=`expr "$$HASMAIN" : '.*\.a\[\(.*\)\].*'` ;\
    $(AR) -d $(LIB) $$MOD && $(RANLIB) $(LIB) ;\
  fi
endef	# StripMain
