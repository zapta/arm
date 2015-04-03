// Protocol related utilities and definitions.

#ifndef PROTOCOL_UTIL_H
#define PROTOCOL_UTIL_H

#include "mbed.h"

namespace protocol {
  extern void protocolPanic(const char* short_message);
  //extern bool is_panic_mode;
}

namespace protocol_util {

// General protocol buffer tag types.
// Per https://developers.google.com/protocol-buffers/docs/encoding.
static const uint8_t kTagTypeVarint = 0;
static const uint8_t kTagTypeFixed64 = 1;
static const uint8_t kTagTypeLenDelimited = 2;
static const uint8_t kTagTypeStartGroup = 3;
static const uint8_t kTagTypeEndGroup = 4;
static const uint8_t kTagTypeFixed32 = 5;

}  // namespace protocol_util

#endif  // PROTOCOL_UTIL_H
