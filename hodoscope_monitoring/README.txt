This monitoring software is composed of two sections:

 - the main macro create_data.cxx mimics the hodoscope acquisition system and produces random data which are then stored in the folder ./data/ with increasing number.
   the data structure is defined according to the hodoscope data format specifications

 - the main macro monitor.cxx is the actual monitoring system, which scans the data folder and creates some plots of interest for the acquisition. The macro turns in a continuous while
   which looks for new files in the folder and reads the completed files to update the plots. So that the data are shown online

Both the main macros use the header functions.h, where the data structure and the data packing and unpacking methods are defined. Everything corresponds to the data format specifications for the
hodoscope.

In order to test the monitoring macro with the real acquisition, one should modify the path to the data folder and the file name convention according to his choice. In the first lines of
monitor.cxx some more details are given.

The Makefile in the folder compiles the two macros with clang++, but the compilation of create_data.cxx is not mandatory to work only with the monitoring.
So it is possible to just compile the monitor.cxx part.

Everything works with root_v6.06.06 and this clang compiler
  Apple LLVM version 8.0.0 (clang-800.0.42.1)
  Target: x86_64-apple-darwin15.6.0
  Thread model: posix

Once the two macros are compiled, just execute them in parallel into two different terminals, simply like
$ ./create_data
$ ./monitor
no matter which one first - the monitoring part is able to wait for data for undefined time


ENJOY!

In case of problems,

m.fontana@ipnl.in2p3.fr
