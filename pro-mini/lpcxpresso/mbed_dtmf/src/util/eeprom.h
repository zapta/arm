#include "util/common.h"

#ifndef LPC11U35_EEPROM_H
#define LPC11U35_EEPROM_H

namespace eeprom {

// Offset in eeprom area where we keep the config record.
// As of Apr 2015, record size is 144 bytes.
const uint32_t kEepromConfigOffset = 0;

// Offset in eeprom for storing the reboot mode enum.
const uint32_t kEepromRebootModeOffset = 0x100;

//extern void IAP_test();

extern bool eeprom_write(uint32_t dst_eeprom_offset, const void* src_ram_address, uint32_t n);
extern bool eeprom_read (uint32_t src_eeprom_offset, void* dst_ram_address, uint32_t n);

}  // namespace eeprom


#endif  // LPC11U35_EEPROM_H
