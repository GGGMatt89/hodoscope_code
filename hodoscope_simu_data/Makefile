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
LIB = create_data.a
AR  = ar -r

#----------------------
# Makefile head section
#----------------------

all:
	make create_data

create_data: create_data.o $(LIB)
	$(CXX) $(CXXFLAGS) -o create_data create_data.o


clean:
	rm -f *.o *.a

cleanall:
	rm -f *.o *.a create_data


#----------------------------------
# Dependencies for the source files
#----------------------------------

CREATE_DATA_CPP_DEP = \
	functions.h \

#--------------------------------
# Compilation of the source files
#--------------------------------

create_data.o : create_data.cxx $(CREATE_DATA_CPP_DEP)
	$(CXX) $(CXXFLAGS) -c create_data.cxx -o create_data.o #$(ROOTFLAGS)


#----------------------------------
# Library dependencies and creation
#----------------------------------

LIB_DEP = \
	create_data.o \

$(LIB) : $(LIB_DEP)
	$(AR) $(LIB) $(LIB_DEP) $(POST_AR)
