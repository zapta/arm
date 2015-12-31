#ifndef EEPROM_H
#define EEPROM_H

namespace eeprom {

static const uint32 system_clock_khz = Chip_Clock_GetSystemClockRate() / 1000;

static const uint32_t kIapRequestSize = 5;
static const uint32_t kIapResponseSize = 5;

typedef uint32_t IapRequest[kIapRequestSize];
typedef uint32_t IapResponse[kIapResponseSize];

typedef void (*IAP)(uint32_t[5], uint32_t[4]);
static const IAP IAP_entry = (IAP) 0x1FFF1FF1;

/** Copy RAM to EEPROM
 *
 *  @param    dst_eeprom_offset		Destination EEPROM address where data bytes are to be written.
 *  @param    src_ram_address		Source RAM address from which data bytes are to be read.
 *  @param    Number_of_Bytes		Number of bytes to be written.
 *  @return   error code: CMD_SUCCESS = 0 | SRC_ADDR_NOT_MAPPED = 4 | DST_ADDR_NOT_MAPPED = 5
 *  Remark: The top 64 bytes of the EEPROM memory are reserved and cannot be written to.
 */
uint32_t Write(uint32_t dst_eeprom_offset, const void* src_ram_address, uint32_t Number_of_Bytes) {
	IapRequest req;
	IapResponse resp;

	req[0] = 61; // EEPROM Write IAPCommand
	req[1] = dst_eeprom_offset; // EEPROM address
	req[2] = reinterpret_cast<uint32_t>(src_ram_address); // RAM address
	req[3] = Number_of_Bytes; // Number of bytes to be read
	req[4] = system_clock_khz; // The minimum CCLK frequency supported is CCLK = 200 kHz.

	// Initialize the Response buffer
	for (uint32_t i = 0; i < kIapResponseSize; i++) {
		resp[i] = 0;
	}

	//	Disable All interrupts before calling IAP
	//	through clearing the exception mask register (PRIMASK).
	__disable_irq();
	// Call IAP
	IAP_entry(req, resp);
	// Re-enable All interrupts
	__enable_irq();

	// Expected status codes are:
	// CMD_SUCCESS = 0 | SRC_ADDR_NOT_MAPPED = 4 | DST_ADDR_NOT_MAPPED = 5
	return resp[0];
}

/** Copy EEPROM to RAM
 *
 *  @param    src_eeprom_offset		Source EEPROM address from which data bytes are to be read.
 *  @param    dst_ram_address		Destination RAM address where data bytes are to be written.
 *  @param    Number_of_Bytes		Number of bytes to be written.
 *  @return   error code: CMD_SUCCESS = 0 | SRC_ADDR_NOT_MAPPED = 4 | DST_ADDR_NOT_MAPPED = 5
 */
uint32_t Read(uint32_t src_eeprom_offset, void* dst_ram_address, uint32_t Number_of_Bytes) {
	IapRequest req;
	IapResponse resp;

	req[0] = 62; // EEPROM Read IAPCommand
	req[1] = src_eeprom_offset; // EEPROM address
	req[2] = reinterpret_cast<uint32_t>(dst_ram_address); // RAM address
	req[3] = Number_of_Bytes; // Number of bytes to be read
	req[4] = system_clock_khz; // The minimum CCLK frequency supported is CCLK = 200 kHz.

	// Initialize the Response buffer
	for (uint32_t i = 0; i < kIapResponseSize; i++) {
		resp[i] = 0;
	}

	//	Disable All interrupts before calling IAP
	//	through clearing the exception mask register (PRIMASK).
	__disable_irq();
	// Call IAP
	IAP_entry(req, resp);
	// Re-enable All interrupts
	__enable_irq();

	// Expected status codes are:
	// CMD_SUCCESS = 0 | SRC_ADDR_NOT_MAPPED = 4 | DST_ADDR_NOT_MAPPED = 5
	return resp[0];
}

}  // namespace eeprom

#endif  // EEPROM_H
