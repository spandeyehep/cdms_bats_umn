# Makefile linking ROOT libraries

CC=gcc  -g
CCFLAGS   =  -pthread -O $(shell root-config --cflags)
ROOTLIBS  =  $(shell root-config --libs)  
LD    =  gcc -g 
LDFLAGS   =  $(shell root-config --ldflags) 

.SUFFIXES: .C .o .cxx

all: CheckOutput CheckSupermerge merge_filelist

%.o: %.C
	$(CC) -c $< $(CCFLAGS)

%.o: %.cxx
	$(CC) -c $< $(CCFLAGS)

CheckOutput:CheckOutput.o 
	$(LD) $(LDFLAGS) $^ $(ROOTLIBS)  -o $@

CheckSupermerge:CheckSupermerge.o 
	$(LD) $(LDFLAGS) $^ $(ROOTLIBS)  -o $@

merge_filelist:merge_filelist.o
	$(LD) $(LDFLAGS) $^ $(ROOTLIBS)  -o $@