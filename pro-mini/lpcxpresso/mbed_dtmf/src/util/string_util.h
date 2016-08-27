#ifndef STRING_UTIL_H
#define STRING_UTIL__H

#include "util/common.h"

namespace string_util {

// Poor man's replacement for snprint "%llu" and "%llx" formats.
//
// Encode a uint64 into an internal buffer and return a pointer
// into the buffer where the encoded string is.
// Both method share the same internal space and each corrupts the
// result of the other.
extern char* uint64ToDecimal(uint64_t v);
extern char* uint64ToHex(uint64_t v);

// Hex accepts lower case and upper case hex. Leading zeros are ok.
// It checks for uint64 overflow.
// Return false if error.
extern bool hexStringToUint64(const char* str, uint64_t* value);
extern bool decimalStringToUint64(const char* str, uint64_t* value);


// Returns true of str matches any of strings. Strings is a null
// terminated.
extern bool matchesStringSet(const char* str, const char* strings[]);

extern void copyString(char *dest, const char *src, size_t dest_size);

}  // namespace string_util

#endif  // STRING_UTIL__H
