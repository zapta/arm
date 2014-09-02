/****************************************************************************
 *   $Id:: main.c 4528 2010-08-25 23:32:22Z nxp41504                        $
 *   Project: NXPISP flash programming tool for Mac
 *
 *   Description:
 *     This file implements a command-line tool to detect LPC1XXX devices in
 *     USB ISP mode and reprogram their firmware in flash memory.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 ****************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <DiskArbitration/DiskArbitration.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFString.h>

// newDiskEvent is called when a new disk is added to the system.
//	It looks up the disk information and determines whether it is an
//	LPC1XXX device in ISP mode or not. If yes, then the code passed
//	to main() is programmed into flash.
//	There are some other bits of logic to test for and remove code
//	protect, as well as compare the binary file sizes to make sure
//	that the code to be downloaded is the same size as the flash
//	memory on the chip that is connected to USB.
void newDiskEvent(DADiskRef disk, void *data)
{
	boolean_t fail = false;
	CFStringRef daDeviceModel, daDeviceVendor, daVolumeName;
	CFURLRef daVolumePath;
	CFDictionaryRef diskDescription = DADiskCopyDescription(disk);
	int newFirmware_fd = (int)data;
	int firmwareBin_fd = 0;

	// If we cannot get the disk attributes then fail.
	if(!diskDescription)
		fail = true;

	if(!fail && !CFDictionaryGetValueIfPresent(diskDescription, CFSTR("DADeviceModel"), (const void **)&daDeviceModel))
	{
		daDeviceModel = 0;
		fail = true;
	}
	if(!fail && !CFDictionaryGetValueIfPresent(diskDescription, CFSTR("DADeviceVendor"), (const void **)&daDeviceVendor))
	{
		daDeviceVendor = 0;
		fail = true;
	}

	// To check whether we have an NXP ISP device, we compare the Device Model and Device Vendor
	if(!fail
	   && CFStringCompare(daDeviceModel, CFSTR("LPC1XXX IFLASH"), kCFCompareEqualTo) == kCFCompareEqualTo
	   && CFStringCompare(daDeviceVendor, CFSTR("NXP"), kCFCompareEqualTo) == kCFCompareEqualTo)
	{
		char firmwareBinPath[1024];
		boolean_t ISP = false, CP = false;

		// Once we have verified that an NXP device is connected in ISP mode, we grab the
		// volume name and volume path. The volume name will be used to check for CRP mode.
		// The volume path will be used to find the firmware.bin file to overwrite.
		if(!fail && !CFDictionaryGetValueIfPresent(diskDescription, CFSTR("DAVolumeName"), (const void **)&daVolumeName))
		{
			daVolumeName = 0;
			fail = true;
		}
		if(!fail && !CFDictionaryGetValueIfPresent(diskDescription, CFSTR("DAVolumePath"), (const void **)&daVolumePath))
		{
			daVolumePath = 0;
			fail = true;
		}

		if(!fail && !CFURLGetFileSystemRepresentation(daVolumePath, true,
										 (unsigned char *)firmwareBinPath, sizeof(firmwareBinPath)))
		{
			firmwareBinPath[0] = 0;
			fail = true;
		}
		// Check buffer size... if okay, then-
		if(strlen(firmwareBinPath) + strlen("/firmware.bin") > sizeof(firmwareBinPath)-1)
			fail = 1;
		if(!fail)
		{
			// Create file path to firmware.bin file
			strcat(firmwareBinPath, "/firmware.bin");
			
			// Check CRP status (volume name) if no CRP then reprogram device
			if(!fail && CFStringCompare(daVolumeName, CFSTR("CRP DISABLD"), kCFCompareEqualTo) == kCFCompareEqualTo)
			{
				int len;
				ISP = true;
				printf("Reprogramming device...\n");
				// Open file _without truncation_
				// If file is truncated/deleted and rewritten, OS-X's FAT driver will reallocate
				// the file blocks in a different order. This will cause the firmware to be
				// scrambled as it is written to flash.
				firmwareBin_fd = open(firmwareBinPath, O_RDWR | O_SHLOCK);
				if(firmwareBin_fd == -1)
				{ // failed to open file...
					firmwareBin_fd = 0;
					fail = true;
				}
				
				// Sanity checks- seek to end of new firmware file to get the size (lseek returns
				// the current offset = length of file if seeking to end)
				len = lseek(newFirmware_fd, 0, SEEK_END);
				// If binary size is at least 4K and less than 1 MB assume it is a good value
				// in addition, make sure the flash size is the same as the incoming binary size
				// Mostly this size check is here to prevent the malloc below from reading in some
				// incorrect huge multi-GB file
				if(!fail && len >= 4096 && len <1024*1024 && len==lseek(firmwareBin_fd, 0, SEEK_END))
				{
					// Allocate a buffer in Mac RAM for our new firmware
					char *buf = malloc(len);
					
					if(!buf)
						fail = true;

					// Return file pointers to the beginning of the files
					if(!fail && lseek(firmwareBin_fd, 0, SEEK_SET) != 0)
						fail = true;
					if(!fail && lseek(newFirmware_fd, 0, SEEK_SET) != 0)
						fail = true;
					
					// Read the new firmware
					if(!fail && read(newFirmware_fd, buf, len) != len)
						fail = true;
					// Write it into the firmware.bin on the LPC1xxx part in ISP mode
					if(!fail && write(firmwareBin_fd, buf, len) != len)
						fail = true;
					if(buf)
						free(buf);
				} else fail = true;
				if(firmwareBin_fd)
					close(firmwareBin_fd);
			} else
				// CRP1 or CRP2 is enabled. We must delete firmware.bin to erase the chip.
				// Then the user must disconnect, power down, reset the device to reset the CRP
				// setting to "No CRP."
				// Then they must reconnect the blank device to finally flash in the new firmware.
				// After the device has been erased, it will come up in ISP mode by default. The
				// user will not have to take any action to force the device to boot in ISP mode.
			if(!fail && (CFStringCompare(daVolumeName, CFSTR("CRP1 ENABLD"), kCFCompareEqualTo) == kCFCompareEqualTo
			   || CFStringCompare(daVolumeName, CFSTR("CRP2 ENABLD"), kCFCompareEqualTo) == kCFCompareEqualTo))
			{
				CP = true;
				printf("A code protected NXP ISP device was found. This device must be erased before\n"
						"it can be reprogrammed. Erasing...\n");
				// Deleting the firmware.bin file sends a signal to the USB ISP code in ROM to
				// erase all of the flash including the code protect settings
				unlink(firmwareBinPath);
			} else
				// There is no cure for CRP3
			if(!fail && CFStringCompare(daVolumeName, CFSTR("CRP3 ENABLD"), kCFCompareEqualTo) == kCFCompareEqualTo)
			{
				printf("An NXP ISP device was found, but it cannot be reprogrammed due to its code protect settings.\n");
			} else {
				// Maybe if the ISP is changed to include new features we would see this error.
				printf("An unexpected volume name was encountered. No changes made.\n");
			}

			// Unmount the ISP disk so the user can unplug/replug it to start the new firmware
			// without the risk of loosing data because the OS had not written it yet.
			DADiskUnmount(disk, kDADiskUnmountOptionDefault, NULL, 0);
			if(!fail)
			{
				// If code protect found, we advise the user to unplug and try again. We also stay
				// "armed" looking for more devices to try to update.
				if(CP)
					printf("Device erased. Please disconnect, reset, and reconnect the device to update firmware.\n");
				// If we flashed something, then we close our new firmware file and quit
				if(ISP)
				{
					printf("NXPISP successfully updated the devices' firmware.\n");
					close(newFirmware_fd);
					exit(0);
				}
			} else {
				// If there is an error we close our firmware file and quit.
				printf("There was an error updating the firmware.\n");
				close(newFirmware_fd);
				exit(-1);
			}

		}
	}
	
	if(diskDescription) CFRelease(diskDescription);
	// Freeing return values from CFGetDictionaryIfPresent causes problems.
	// Those must be non-reference counted pointers into subsets of the dictionary returned by
	// DADiskCopyDescription.
}

int main (int argc, const char * argv[])
{
	int newFirmware_fd;
	const char *filename = "firmware.bin";
	
	if(argc != 2)
	{
		// If no argument passed, we print out the arg template but continue anyway using
		// a default file name of firmware.bin
		fprintf(stderr, "NXPISP <filename>\n");
	} else {
		filename = argv[1];
	}

	printf("NXPISP will flash %s into the next NXP ISP device connected to this mac.\n", filename);
	newFirmware_fd = open(filename, O_RDONLY | O_SHLOCK);
	
	if(newFirmware_fd==-1)
	{
		fprintf(stderr, "NXPISP: Cannot open firmware to be downloaded into NXP ISP devices.\n");
		exit(-1);
	}
		
	DASessionRef daSession = DASessionCreate(kCFAllocatorDefault);

	// Register a callback to detect the insertion of new devices
	DARegisterDiskAppearedCallback(daSession, NULL, newDiskEvent, (void *)newFirmware_fd);
	
	DASessionScheduleWithRunLoop(daSession, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	
	// Enter our main loop
	CFRunLoopRun();
	
	CFRelease(daSession);
	return 0;
}
