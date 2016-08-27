
#include "util/string_util.h"

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

bool matchesStringSet(const char* str, const char* strings[]) {
  for (const char** p = strings; *p; p++) {
    if (strcmp(*p, str) == 0) {
      return true;
    }
  }
  return false;
}

bool hexStringToUint64(const char* str, uint64_t* value) {
  uint64_t result = 0;
  for(;;) {
    // Done if reached the null terminator.
    // Treading zero length strings as a valid zero value.
    const char c = *(str++);
    if (!c) {
      *value = result;
      return true;
    }
    // Check or overflow.
    if (result & (static_cast<uint64_t>(0xf) << 60)) {
      return false;
    }
    // Add new digit.
    uint32_t hex_digit;
    if (c >= '0' && c <='9') {
      hex_digit = (c - '0');
    } else if (c >= 'a' && c <= 'f') {
      hex_digit = c - ('a' - 10);
    } else if (c >= 'A' && c <= 'F') {
         hex_digit = c - ('A' - 10);
    } else {
      // Not a hex digit.
      return false;
    }
    result = (result << 4) + hex_digit;
  }
}

static const uint64_t kMaxUint64 = 0xffffffffffffffffll;

bool decimalStringToUint64(const char* str, uint64_t* value) {
  uint64_t result = 0;
  for(;;) {
    // Done if reached the null terminator.
    // Treading zero length strings as a valid zero value.
    const char c = *(str++);
    if (!c) {
      *value = result;
      return true;
    }
    // Make sure we have room to one zero.
    if (result > (kMaxUint64 / 10)) {
      return false;
    }
    result = result * 10;

    // Add new digit.
    if (c < '0' || c > '9') {
      // Not a decimal digit.
      return false;
    }
    const uint32_t decimal_digit = (c - '0');
    const uint64_t temp = result + decimal_digit;
    if (temp < result) {
      // Last digit caused an overflow. The test above that we can
      // multiply by 10 without an overflow is not sufficient to
      // detect overflow.
      return false;
    }
    result = temp;
  }
}

void copyString(char *dest, const char *src, size_t dest_size) {
  size_t n = strlen(src);
  if (n >= dest_size) {
    n = dest_size - 1;
  }
  memcpy(dest, src, n);
  dest[n] = '\0';
}

}  // namespace string_util
