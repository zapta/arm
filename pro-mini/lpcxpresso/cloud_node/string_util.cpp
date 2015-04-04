
#include "string_util.h"
//#include "debug.h"
//#include "esp8266.h"
//#include "protocol_util.h"

namespace string_util {

// Size is sufficient to contain a uint64 in decimal or hex format including
// a null terminator.
static char uint64_to_str_buffer[20+1];

char* uint64ToDecimal(uint64_t v) {
  char* p = uint64_to_str_buffer + sizeof(uint64_to_str_buffer);
  *(--p) = '\0';
  for (bool first = true; v || first; first = false) {
    uint32_t digit = v % 10;
    const char c = '0' + digit;
    *(--p) = c;
    v = v / 10;
  }
  return p;
}

char* uint64ToHex(uint64_t v) {
  char* p = uint64_to_str_buffer + sizeof(uint64_to_str_buffer);
  *(--p) = '\0';
  for (bool first = true; v || first; first = false) {
    uint32_t digit = v & 0xf;
    const char c = (digit < 10)
        ? ('0' + digit)
        : ('a' - 10 + digit);
    *(--p) = c;
    v = v >> 4;
  }
  return p;
}

void appendChar(char* bfr, int bfr_size, uint8_t c) {
  const int n = strlen(bfr);
  if (n < bfr_size - 1) {
    bfr[n] = c;
    bfr[n+1] = '\0';
  }
}


}  // namespace string_util
