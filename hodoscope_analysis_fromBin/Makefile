#---------------------------------------
# Resources section - Symbol definitions
#---------------------------------------
ROOTFLAGS = $(shell root-config --cflags)
ROOTLIBS  = $(shell root-config --libs)
#THREAD = -lpthread

CXX := g++
#CXX := clang++
CXXFLAGS := -std=c++11 #-stdlib=libstdc++
#CXXFLAGS := -std=c++11 -stdlib=libc++


C++ = clang -O
LIB = analysis_fromBin.a
AR  = ar -r

#----------------------
# Makefile head section
#----------------------

all:
	make analysis_fromBin

analysis_fromBin: analysis_fromBin.o $(LIB)
	$(CXX) $(CXXFLAGS) -o analysis_fromBin analysis_fromBin.o $(ROOTFLAGS) $(ROOTLIBS)

clean:
	rm -f *.o *.a

cleanall:
	rm -f *.o *.a analysis_fromBin


#----------------------------------
# Dependencies for the source files
#----------------------------------

ANALYSIS_FROMBIN_CPP_DEP = \
	functions.h \

#--------------------------------
# Compilation of the source files
#--------------------------------

analysis_fromBin.o : analysis_fromBin.cxx $(CREATE_DATA_CPP_DEP)
	$(CXX) $(CXXFLAGS) -c analysis_fromBin.cxx -o analysis_fromBin.o $(ROOTFLAGS)

#----------------------------------
# Library dependencies and creation
#----------------------------------

LIB_DEP = \
	analysis_fromBin.o \

$(LIB) : $(LIB_DEP)
	$(AR) $(LIB) $(LIB_DEP) $(POST_AR)
