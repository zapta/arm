ARM PRO MINI
============

### OVERVIEW

ARM PRO MINI is a small barebone open source ARM M0 microcontroller board that is great for 
quick prototyping and as a starting point for your own ARM based custom designs. 
It was designed and named after the venerable Arduino Pro Mini and it is an excellent 
stepping stone for makers and hobbyists 'graduating' from Arduino to the ARM architecture.

![](www/arm_pro_mini.jpg)


### Highlights

* Prototyping friendly. Compatible with standard soldieries breadboards and can be soldered to a standard 0.1” proto board.
* Simple and easy to understand barebone design. Customize for your own PCB design by selecting the portions of the circuit you need.
* Single package install of the free toolchain and IDE (NXP Eclipse/LPCXpresso).
* Full support of Windows, Mac OSX and Linux.
* Easy firmware upgrade using a simple file drag and drop. Programmers and adapters are not required, even if you just built your own board.
* Supports optional debuggers such as the $30 OM13014,598 (for single stepping and full debugging capabilities).
* I/O library and a hello world example (with serial printing over USB, parallel port io, blinking LED, and timing).
* Liberal open source license (commercial use OK and free, sharing and attribution not required).


### Specification

Attribute | Value
:--------- | :-----
MCU | NXP ARM M0 LPC11U35FHI33/501
Memory | 64K flash, 8K RAM, 4K EEPROM
Speed | 48Mhz
Core voltage | 3.3V   (3.3V LDO included)
MCU Package | QFN 5x5mm, 0.5mm pitch, 32 pads + ground tab.
Dimensions | 1.5” x 0.775”  (38.1mm x 19.7mm)
PCB Layers | 2
PCB Thickness | 1mm recommended. (standard 1.6mm is also OK).
PCB file format | Eagle + PDF + Gerber files.
SMT Technology | 0402, QFN 0.5mm pitch.
Power options | USB, ext 3.3v, ext ??-?? (3.3V LDO included) 
Header pins | 2x15 DIP, 0.7” raw spacing. Access to all MCU's power and I/O pins.
USB Connector | Micro B
Debugger Connector | SWD 2x5 pin header, 0.5” pitch
Crystal | 12Mhz (for 48Mhz MCU operation).
LEDS | Power (blue), USB status (green), Application (red), 
Switches | Reset, ISP.
Recomanded IDE | NXP LPCXpresso (eclipse based, free)


### Quick Start 1 - Uploading a Program

This section will teach you how to load new firmware on your board. It is done by switching the board to the USB/ISP virtual disk mode and copying the new binary file.

1. Connect the ARM PRO MINI board to a USB port of a Max OSX, Linux or Windows computer.
2. Restart the board in the USB/ISP mode by performing the following sequence (with some practice, can be done with a simple 'roll' of one finger):
    * Press and hold the RST button.
    * Press and hold the ISP button.
    * Release the RST button.
    * Release the ISP button.
3. The ARM PRO MINI will mount itself on your computer as an external disk with a single file named firmware.bin.
4. Copy one of the two provided hello world image files over the *firmware.bin* file. Note: on Mac OSX drag and drop does not work, instead using the cp shell command: *cp hello_world_fast_blink.bin "/Volumes/CRP DISABLD/firmware.bin"*
5. Press the RST button of the ARM PRO MINI to restart it in the RUN mode. The image file you just loaded should start running and the red LED on the ARM PRO MINI should blink.
6. Repeat the process with the other image file *hello_world_slow_blink.bin*  and notice how the blinking rate changes.

For more information about the USB bootloader see NXP's [application note AN11305](resources/lpc_bootloader/AN11305v.1.pdf).



### Quick Start 2 - Using the USB/Serial port.

This section will teach you how to view on your computer seial data output from the ARM PRO MINI. It is useful for example to communicate with a program running on the ARM PRO MINI or to debug a program you write using print messages.

1. Load and run a hello world binary image as described above. Make sure the red LED blinks.
2. Identify the serial port on your computer. 
    * On Mac OSX:  run _ls /dev/tty.usb*_, the port name looks like */dev/tty.usbmodemNXP-71*.
    * On Linux: TBD
    * On Windows: TBD
3. Use a terminal emulator to print the text received on the serial port you identified above.
    * On Mac OSX: run the command screen <port_name>. For example *screen /dev/tty.usbmodemNXP-71*
    * On Linux: TBD
    * On Windows: TBD

The output will look like this:
~~~
...
Hello world: 4, 1500177
Hello world: 5, 1800178
Hello world: 6, 2100178
Hello world: 7, 2400178
Hello world: 8, 2700177
...
~~~




### Quick Start 3 - Installing the IDE and compiling Hello World.

This section will teach you how to install the NXP LPCXpresso IDE and how to setup and compile a project for the ARM PRO MINI.

TO BE CONTINUED...


