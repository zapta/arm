TODO list for Cloud Node

**SCHEMATIC**
* Create schematic of integrated MCU, ESP8266, OLED


**LAYOUT**


** Enclosure


**SOFTWARE**
* Fix the HH.MM.DD display (negative values)
* Solve the Lua syncrhonization issue between output from onX events and output from the command line (currently just the prompt since the echo is disabled)
* Cleanup interrupt driven RX.
* Schedule sending using a readyToSend() method in protocol.tx to avoid overlapping sends (e.g. heartbeat).
* Better handling of protocolPanic. Currently it soemtimes doesn't close connection.
* Verify that we have sufficient RAM margin.

* Store configruation and credentials in MCU EEPROM.
* Revisit buffer sizes.
* Have a linux command line tool for configruation (id, registration, ssid, etc).
* Decouple from mbed USSerial and Serial. This wills save at least 15K of flash.
* Do something cool with it.
* Move the C++ code into the ESP8266 IC (no external MCU).
* Intergrate the USB/Serial with the Cloud Node firmaware (e.g. to reflash the ESP8266)
* Send upstream messages (e.g. from a sensor).
* Optimzie power consumption when running on a battery (minimize transmissions, sleep mode, etc).

**DOCUMENTATION**


**MISCELLANEOUS**




