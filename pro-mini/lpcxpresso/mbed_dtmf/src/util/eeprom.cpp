

#include "util/eeprom.h"
#include "util/common.h"

namespace eeprom {

// TODO: refactor this out to a lpc_iap.h/cpp files.
static const uint32_t kIapRequestSize = 5;
static const uint32_t kIapResponseSize = 5;

typedef uint32_t IapRequest[kIapRequestSize];
typedef uint32_t IapResponse[kIapResponseSize];

#ifndef TARGET_LPC11U35_501
#error "Should verify MCU compatibility"
#endif
typedef void (*IAP)(uint32_t[5], uint32_t[4]);
static const IAP IAP_entry = (IAP)0x1FFF1FF1;


bool eeprom_write (uint32_t dst_eeprom_offset, const void* src_ram_address, uint32_t n) {
  IapRequest req;
  IapResponse resp;

  req[0] = 61;
  req[1] = dst_eeprom_offset;
  req[2] = reinterpret_cast<uint32_t>(src_ram_address);
  req[3] = n;
  req[4] = 48000;   // sys clock / 1000. Define a const.

  for (uint32_t i = 0; i < kIapResponseSize; i++) {
     resp[i] = 0;
   }

  IAP_entry(req, resp);

  // See LPC11U35 manual for IAP status codes.
  return !resp[0];
}

bool eeprom_read (uint32_t src_eeprom_offset, void* dst_ram_address, uint32_t n) {
  IapRequest req;
  IapResponse resp;

  req[0] = 62;
  req[1] = src_eeprom_offset;
  req[2] = reinterpret_cast<uint32_t>(dst_ram_address);
  req[3] = n;
  req[4] = 48000;   // sys clock / 1000. Define a const.

  for (uint32_t i = 0; i < kIapResponseSize; i++) {
     resp[i] = 0;
   }

  IAP_entry(req, resp);

  // See LPC11U35 manual for IAP status codes.
  return !resp[0];
}

}  // namespace eeprom

