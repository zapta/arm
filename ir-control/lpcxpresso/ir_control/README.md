Dec 2015
Exporting a project from mbed online ide to LPCXpresso 8.x

1. Changing library referneces to _nano
* Open the file ${project}/mbed/TARGET_LPC11U35_501/TOOLCHAIN_GCC_CR/LPC11U35.ld
* In the GROUP statement at the begining of the file, change "libc_s.a libstdc++_s.a" to 
   "libc_nano.a libstdc++_nano.a"  (that is, change _s.a to _nano.a).
* Save the file.

2. Enable checksum generation in the generted .bin file
* Open project properties.
* In the C/C++ Build section, select on the Settings entry.
* On the right hand, select the Build Steps tab
* Edit the Post-build steps Command to remove the '#' (comment mark) before the checksum command.

3. Select the designed optimization setting.
* Open project properties. 
* In the C/C++ Build section, select on the Settings entry.
* On the right hand, select the Tool Settings tab.
* Select MCU C++ Compiler / Optimization and set the desired optimization level.


TODO: may need to set also the debugging and optimizations to have everything working well. Revisit.




