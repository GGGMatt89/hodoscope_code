#---------------------------------------
# Resources section - Symbol definitions
#---------------------------------------
ROOTFLAGS = $(shell root-config --cflags)
ROOTLIBS  = $(shell root-config --libs)


CXX := clang++
#CXXFLAGS := -std=c++11 -stdlib=libstdc++
CXXFLAGS := -std=c++11 -stdlib=libc++

#----------------------
# Makefile head section
#----------------------

all:
	make convertToROOT

convertToROOT: convertToROOT.o $(LIB)
	$(CXX) $(CXXFLAGS) -o convertToROOT convertToROOT.o $(ROOTFLAGS) $(ROOTLIBS)

clean:
	rm -f *.o

cleanall:
	rm -f *.o *.a convertToROOT

#----------------------------------
# Dependencies for the source files
#----------------------------------

convertToROOT_CPP_DEP = \
	functions.h \

#--------------------------------
# Compilation of the source files
#--------------------------------

convertToROOT.o : convertToROOT.cxx $(convertToROOT_CPP_DEP)
	$(CXX) $(CXXFLAGS) -c convertToROOT.cxx -o convertToROOT.o $(ROOTFLAGS)
