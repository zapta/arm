ARM PRO MINI
============

### OVERVIEW

Phone Finder is simple system that allows familty members to locate their cell phones.
The system works with an arbitrary number of Amazon Dash buttons that are placed 
in key locations such as near the exit door. When a Dash button is pressed, the system
dials the phone number that associated with that button, causing the respective cell
phone to ring.

The system uses a Phone Finder box that bridges between the Wifi connections from
the Dash buttons and the phone landline cord. The Phone Finder box acts as a Wifi 
access point that accepts Wifi connections directly from the Dash buttons and does
no require a local Wifi network or a connection to the internet.

![](www/phone_finder_1.jpg)

![](www/phone_finder_2.jpg)

![](www/phone_finder_3.jpg)

[Schematic (PDF)](https://github.com/zapta/arm/blob/master/phone-finder/eagle/phone-finder-schematic.pdf)

This design is in the public domain and is provided AS IS. I am not an electronic or telephony engineer. All source files including eagle, LPCXpresso project, C++ source files and openscad 3D model source files are included.

<br>

### Design notes (assorted order)

* The PCB include an ARM mcu, a Wifi module and a phone line dialer. 
* The MCU design is based on the Arm Pro Mini. 
* The firmware is mbed based and was developed using NPX LPCXpresso (see ARM Pro Mini documentation)
* Loading the firmaware is done via the USB connector, similar to ARM Pro Mini.
* The enclosure and the LED mount are 3D printed. STL files and openscad source files are availabe here.
* To change the phone numbers you will need change the source files and build. Currntly there is no soft EEPROM based configuration.
* The DTFM is generated using two 16 bit counters of the ARM MCU. They generate square waves that are filtered out by a simple RC filter, the coupling transformer and the phone line itself.
* For simplicty, the coupling transformer was not decoupled from DC on both its primary and secondary. Howerver, it seems to work well.
* When assembling the PCB, the orientation of the transformer does matter. Make sure the primary is at the PCB side marked with 'P'.
* If you want to experiment talking directly to the wifi module, build and load the wifi-serial project. This will make the MCU acting as a USB to serial (CDC) adapter.
* For antenna I just soldered a 2$ AWG solid wire (strip at the end for the solder joint). If you want to experiment with different antennas, you can use the wifi module's s/n figure to compare configurations (use the serial-wifi firmware above).
* The PCB is attached to the enclosure base using Scotch Mounting Tape.
* The firmware comes with the default AP SSID 'dashrx' and the password 'dashdash' (password must have at least 8 chars). Change the firmware to customize.
* To make a button connecting to the AP, first set it with a real wifi AP connected to the internet. Set its SSID and password to the ones you want to use, and perform the Amazon Dash button setting using that real AP. Abort the process at the point where you need to select a product. Make sure to change the SSID of the real AP to other than your SSID of choice such that it doesn't interfer with the phone finder.
* Once your button(s) connect successuly to your phone finder (connect the USB connector to a computer, the phone finder acts as a  CDC serial port and dumps diagnostic data), add the mac address of those buttons to the button table in the firmware. 

<br>

If I will find time, I plan to design a lower cost version of the Phone Finder, replacing the ARM mcu and Wifi Module with an ESP8266/Arduino.


