autoisp
=====================
This project contains a USB ISP (In System Programming) example for an
NGX board containing the LPC11U37/401 MCU. with USB jack.

When downloaded to the board and executed, the USB mass-storage-device
bootloader in ROM will be started. After a 12 second delay, the MCU will
reset and begin executing the new code that was downloaded.
 
The project makes use of code from the following library project:
- CMSISv2p00_LPC11Uxx
- LPC11Uxx_Driver_Lib

This library project must exist in the same workspace in order
for the project to successfully build.
