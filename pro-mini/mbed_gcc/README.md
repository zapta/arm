A sample mbed based hello world program for the ARM PRO MINI that is 
built using command line gcc.

Note: this process was tested on a Mac OS X with a LPCXpresso 8.0.0 installed.

To build:
* Make sure you have the arm gcc tool chain installed on your computer. For example, 
  installing the LPCXpresso IDE also installs the gcc tool chain.
* Make sure the NXP 'checsum' tool is intalled on your computer. For example, 
  installing the LPCXpresso IDE also installs the checksum utility.
* Edit the Makefile file and make sure the pathes to the gcc binaries and the checksum tool are correct.
* Run 'make clean' to delete all temp files, if any.
* Run 'make all' to generate the .bin file which can be uploaded to the ARM PRO MINI.
