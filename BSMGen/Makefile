#ROOT
ROOTCFLAGS    = $(shell $(ROOTSYS)/bin/root-config --cflags)
ROOTLIBS      = $(shell $(ROOTSYS)/bin/root-config --libs) -lGui -lCore -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lpthread -lm -ldl
ROOTGLIBS     = $(shell $(ROOTSYS)/bin/root-config --glibs)

#PYTHIA
PYTHIADIR     = $(PWD)/../pythia/
LIBDIRARCH    = $(PYTHIADIR)lib/

#HEPMC
HEPMCDIR      = $(PWD)/../hepmc/build/
HEPMCLIB      = $(HEPMCDIR)lib/

FXX           = gfortran 
CXX           = g++ 
CXXFLAGS      = -fPIC -Wno-deprecated -O -ansi -D_GNU_SOURCE -O2
LD            = g++ 
LDFLAGS       = -g
SOFLAGS       = -shared

ARCH         := $(shell root-config --arch)
PLATFORM     := $(shell root-config --platform)

NGLIBS         = $(ROOTGLIBS) 
gGLIBS          = $(filter-out -lNew, $(NGLIBS))

CXXFLAGS      += $(ROOTCFLAGS)
CXXFLAGS      += $(FASTJETFLAGS)
LIBS           = $(ROOTLIBS) 

NGLIBS         = $(ROOTGLIBS) 
NGLIBS        += $(FASTJETLIBS)
GLIBS          = $(filter-out -lNew, $(NGLIBS))

INCLUDEDIR       = ./include
SRCDIR           = ./src/
OUTLIB	         = ./lib/

.SUFFIXES: .cc,.C, .hh
.PREFIXES: ./lib/

all:  lib GenPythia STDHEPConverter LHEGen LHEGenMLM SMGen SUSYDecay

lib: 	$(OUTLIB)GenTreeColumn.o \
	$(OUTLIB)GenTree.o \
	$(OUTLIB)STDHEP.o \
	$(OUTLIB)GenCandidateFiller.o \
	$(OUTLIB)EventFilter.o

$(OUTLIB)GenTreeColumn.o: $(SRCDIR)GenTreeColumn.cc
	$(CXX) $(CXXFLAGS) -c -I$(INCLUDEDIR) -o $(OUTLIB)GenTreeColumn.o $<

$(OUTLIB)GenTree.o: $(SRCDIR)GenTree.cc  $(OUTLIB)GenTreeColumn.o 
	$(CXX) $(CXXFLAGS) -c -I$(INCLUDEDIR) -o $(OUTLIB)GenTree.o $<

$(OUTLIB)GenCandidateFiller.o: $(SRCDIR)GenCandidateFiller.cc $(OUTLIB)GenTree.o
	$(CXX) $(CXXFLAGS) -c -I$(INCLUDEDIR) -I$(HEPMCDIR)include/  \
	-o $(OUTLIB)GenCandidateFiller.o $<

$(OUTLIB)STDHEP.o: $(SRCDIR)STDHEP.cc $(OUTLIB)GenTree.o $(OUTLIB)GenCandidateFiller.o
	$(CXX) $(CXXFLAGS) -c -I$(INCLUDEDIR) -I$(HEPMCDIR)include/ -o	$(OUTLIB)STDHEP.o $<

$(OUTLIB)EventFilter.o: $(SRCDIR)EventFilter.cc 
	$(CXX) $(CXXFLAGS) -c -I$(INCLUDEDIR) -I$(PYTHIADIR)include/ -o	$(OUTLIB)EventFilter.o $<

GenPythia:$(SRCDIR)GenPythia.C 
	$(CXX) $(CXXFLAGS) -o GenPythia $< \
	-I$(INCLUDEDIR) \
	-I$(HEPMCDIR)include/ \
	-I$(PYTHIADIR)include/ \
	$(OUTLIB)/GenTree.o $(OUTLIB)/GenTreeColumn.o $(OUTLIB)/GenCandidateFiller.o $(OUTLIB)/EventFilter.o \
	$(LIBS) \
	-L$(LIBDIRARCH) -lpythia8  -lLHAPDF   \
	-L$(HEPMCLIB) -lHepMC \
	$(GLIBS) $ 

SMGen:$(SRCDIR)SMGen.C 
	$(CXX) $(CXXFLAGS) -o SMGen $< \
	-I$(INCLUDEDIR) \
	-I$(HEPMCDIR)include/ \
	-I$(PYTHIADIR)include/ \
	$(OUTLIB)/GenTree.o $(OUTLIB)/GenTreeColumn.o $(OUTLIB)/GenCandidateFiller.o \
	$(LIBS) \
	-L$(LIBDIRARCH) -lpythia8 -lLHAPDF  \
	-L$(HEPMCLIB) -lHepMC  \
	$(GLIBS) $ 

STDHEPConverter:$(SRCDIR)STDHEPConverter.C
	$(CXX) $(CXXFLAGS) -o STDHEPConverter $< \
	-I$(INCLUDEDIR) \
	-I$(HEPMCDIR)include/ \
	-I$(PYTHIADIR)include/ \
	$(OUTLIB)/GenTree.o $(OUTLIB)/GenTreeColumn.o $(OUTLIB)/GenCandidateFiller.o $(OUTLIB)/STDHEP.o \
	$(LIBS) \
	-L$(LIBDIRARCH) -lpythia8 -lLHAPDF  \
	-L$(HEPMCLIB) -lHepMC \
	$(GLIBS) $ 

LHEGen:$(SRCDIR)LHEGen.C 
	$(CXX) $(CXXFLAGS) -o LHEGen $< \
	-I$(INCLUDEDIR) \
	-I$(HEPMCDIR)include/ \
	-I$(PYTHIADIR)include/ \
	$(OUTLIB)/GenTree.o $(OUTLIB)/GenTreeColumn.o $(OUTLIB)/GenCandidateFiller.o $(OUTLIB)/STDHEP.o \
	$(LIBS) \
	-L$(LIBDIRARCH) -lpythia8 -lLHAPDF  \
	-L$(HEPMCLIB) -lHepMC  \
	$(GLIBS) $ 

LHEGenMLM:$(SRCDIR)LHEGenMLM.C 
	$(CXX) $(CXXFLAGS) -o LHEGenMLM $< \
	-I$(INCLUDEDIR) \
	-I$(HEPMCDIR)include/ \
	-I$(PYTHIADIR)include/ \
	$(OUTLIB)/GenTree.o $(OUTLIB)/GenTreeColumn.o $(OUTLIB)/GenCandidateFiller.o $(OUTLIB)/STDHEP.o \
	$(LIBS) \
	-L$(LIBDIRARCH) -lpythia8 -lLHAPDF  \
	-L$(HEPMCLIB) -lHepMC  \
	$(GLIBS) $ 

ZJetsGen:$(SRCDIR)ZJetsGen.C 
	$(CXX) $(CXXFLAGS) -o ZJetsGen $< \
	-I$(INCLUDEDIR) \
	-I$(HEPMCDIR)include/ \
	-I$(PYTHIADIR)include/ \
	-L$(LIBDIRARCH) -lpythia8 -lLHAPDF  \
	-L$(HEPMCLIB) -lHepMC  \
	$(GLIBS) $ 

SUSYDecay:$(SRCDIR)SUSYDecay.C
	$(CXX) $(CXXFLAGS) -o SUSYDecay $< \
	-I$(INCLUDEDIR) \
	-I$(HEPMCDIR)include/ \
	-I$(PYTHIADIR)include/ \
	-L$(LIBDIRARCH) -lpythia8 -lLHAPDF  \
	-L$(HEPMCLIB) -lHepMC  \
	$(GLIBS) $

clean:
	rm -f $(OUTLIB)*.o 
	rm -f GenPythia LHEGen SUSYDecay STDHEPConverter SMGen SLHAGen
