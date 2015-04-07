TODO list for Cloud Node

**SCHEMATIC**
* Create schematic of integrated MCU, ESP8266, OLED


**LAYOUT**


** Enclosure


**SOFTWARE**
* Make the display a state machine, one display slive per polling loop (take s~3ms).
* Cleanup the experimental pollings.
* Break the esp8266 tx_polling into multiple polling slices using a state machine.
* Verify that we have sufficient RAM margin.
* Refactor the protocol to a seperate library with clean API.
* Cleanup interrupt driven RX.
* Reset the ESP8266 using an output pin of the MCU (each time establishing a connection?)
* Add to the uart IRQ RX fifo a flag indicating it was full and raise a protocol panic when this happens.

* If got stuck for too long in LOGIN state raise protocol panic.
* On Windows, interaction between USBSerial/CDC and the MCU's programs (loosing chars on login?).
* Solve the Lua syncrhonization issue between output from onX events and output from the command line (currently just the prompt since the echo is disabled)
* Schedule sending using a readyToSend() method in protocol.tx to avoid overlapping sends (e.g. heartbeat).
* Better handling of protocolPanic. Currently it soemtimes doesn't close connection.

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




