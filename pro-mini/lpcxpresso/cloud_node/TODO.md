TODO list for Cloud Node

**SCHEMATIC**
* Create schematic of integrated MCU, ESP8266, OLED


**LAYOUT**


** Enclosure


**SOFTWARE**
* Fix the HH.MM.DD display (negative values)
* Store configruation and credentials in MCU EEPROM.
* Cleanup interrupt driven RX.
* Revisit buffer sizes.
* Have a string pool for events.
* Have the event structs in a single C union.
* Have a linux command line tool for configruation (id, registration, ssid, etc).
* Decoupel from mbed USSerial and Serial. This wills save at least 15K of flash.
* Do something tool with it.
* Move the C++ code into the ESP8266.
* Intergrate the USB/Serial with the Cloud Node firmaware (e.g. to reflash the ESP8266)
* Send upstread messages (e.g. from a sensor).

**DOCUMENTATION**


**MISCELLANEOUS**




