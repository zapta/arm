#ifndef STRING_UTIL_H
#define STRING_UTIL__H

#include "mbed.h"

namespace string_util {

// Poor man's replacement for snprint "%llu" and "%llx" formats.
//
// Encode a uint64 into an internal buffer and return a pointer
// into the buffer where the encoded string is.
// Both method share the same internal space and each corrupts the
// result of the other.
extern char* uint64ToDecimal(uint64_t v);
extern char* uint64ToHex(uint64_t v);

extern void appendChar(char* bfr, int bfr_size, uint8_t c);

}  // namespace string_util

#endif  // STRING_UTIL__H
