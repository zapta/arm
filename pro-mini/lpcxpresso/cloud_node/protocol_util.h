// Internal protocol related definitions and functionalities that
// are common for the tx and rx modules.

#ifndef PROTOCOL_UTIL_H
#define PROTOCOL_UTIL_H

#include "mbed.h"

// We declare this protocol function here so it can also be called
// by the protocol tx and rx modules.
namespace protocol {
  extern void protocolPanic(const char* short_message);
}

namespace protocol_util {

// General protocol buffer tag types.
// Per https://developers.google.com/protocol-buffers/docs/encoding.
static const uint8_t kTagTypeVarint = 0;  // ints, enums, bools.
static const uint8_t kTagTypeFixed64 = 1;  // not used
static const uint8_t kTagTypeLenDelimited = 2;  // strings, sub messages
static const uint8_t kTagTypeStartGroup = 3;  // not used
static const uint8_t kTagTypeEndGroup = 4;  // not used
static const uint8_t kTagTypeFixed32 = 5;  // not used

// These track how many top level messages we sent and how many we received.
// They are used for the handshake aspect of the protocol.
extern uint32_t out_messages_counter;
extern uint32_t in_messages_counter;


}  // namespace protocol_util

#endif  // PROTOCOL_UTIL_H
