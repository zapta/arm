// Implementation of protobuff.h.
//
// Resources:
// https://github.com/google/protobuf/blob/master/src/google/protobuf/io/coded_stream.cc
// https://github.com/crosswalk-project/chromium-crosswalk/blob/master/google_apis/gcm/protocol/mcs.proto
// https://developers.google.com/protocol-buffers/docs/encoding
// http://en.wikipedia.org/wiki/Protocol_Buffers
// https://github.com/ChromiumWebApps/chromium/tree/master/google_apis/gcm/base

#include "proto_tx.h"
#include "debug.h"
#include "esp8266.h"
#include "proto_util.h"

namespace proto_tx {

static const char kDomain[] = "mcs.android.com";

static char tmp_buffer[30];

static char bfr[20+1];

static char* uint64ToDecimal(uint64_t v) {
  char* p = bfr + sizeof(bfr);
  *(--p) = '\0';
  for (bool first = true; v || first; first = false) {
    uint32_t digit = v % 10;
    const char c = '0' + digit;
    *(--p) = c;
    v = v / 10;
  }
  return p;
}

static char* uint64ToHex(uint64_t v) {
  char* p = bfr + sizeof(bfr);
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


// ---------- Upstream

//tatic bool is_panic_mode = false;

// Total bytes written so far.
static uint32_t total_bytes_written = 0;

// When true, we don't actually send out data, just incrementing total_bytes_written.
// Useful to determine message size before acutally sending it in a second pass.
static bool is_counting_only_mode = false;

//bool setCountingOnlyMode(bool new_value) {
//  bool old_value = is_counting_only_mode;
//  is_counting_only_mode = new_value;
//  return old_value;
//}

//uint32_t totalBytesWritten() {
//  return total_bytes_written;
//}

// We flush once we get this filled out. Possibly the ESP8266 ships each
// flushed buffer in a separate packet so we don't want this to be
// too small.
// TODO: does the esp8266/Lua uses Nagle's algorithm? If so we can reduce the
// size here.
//static uint8_t write_buffer[30];
//tatic const int kWriteBufferMaxSize = sizeof(write_buffer);
// The current number of written bytes in the buffer.
//static int write_buffer_size = 0;

// This is the underlying raw output function.
// Serialized size = 1.
void writeRawByte(uint8_t b) {
  if (!is_counting_only_mode) {
    //if (write_buffer_size >= kWriteBufferMaxSize) {
    //  flushWrites();
    //}
    esp8266::tx_fifo.putByte(b);

    //write_buffer[write_buffer_size++] = b;
      debug.printf("TX[%02x ");

    if (b == '\r') {
          debug.printf("CR");
        } else if (b == '\n') {
          debug.printf("LF");
        } else {
          debug.printf("%c", b);
        }


    debug.printf("]\n");

  }
  total_bytes_written++;
}


//void writeRawByte(uint8_t b) {
//  if (!is_counting_only_mode) {
//    esp8266::rx_fifo.putByte(b);
////    if (write_buffer_size >= kWriteBufferMaxSize) {
////      flushWrites();
////    }
////    write_buffer[write_buffer_size++] = b;
//    debug.printf("XX[%02x]\n", b);
//  }
//  total_bytes_written++;
//}


// Send given bytes.
void sendProtocolRawBytes(const uint8_t* data, uint32_t size) {
  while (size--) {
    writeRawByte(*data++);
  }
}

//void flushWrites() {
//  if (write_buffer_size > 0) {
//    if (!is_panic_mode) {
//      esp8266::tx(write_buffer, write_buffer_size);;
//    }
//    debug.printf("Flashing %d bytes%s\n", write_buffer_size, is_panic_mode ? " (PANIC)" : "");
//    write_buffer_size = 0;
//  }
//}

// Serialize a varint.
void writeVarint(uint64_t value) {
  while (value > 0x7F) {
    writeRawByte((static_cast<uint8_t>(value) & 0x7F) | 0x80);
    value >>= 7;
  }
  writeRawByte(static_cast<uint8_t>(value) & 0x7F);
}

// Serialize a field tag.
void writeTag(uint32_t tag_num, uint8_t tag_type) {
  writeVarint(tag_num << 3 | tag_type);
}

// Serialize a varint field of given tag. Can be used for
// ints and enums.
void writeVarintField(uint32_t tag_num, uint64_t value) {
  writeTag(tag_num, proto_util::kTagTypeVarint);
  writeVarint(value);
}

// Serialize a bool field of given tag.
void writeBoolField(uint32_t tag_num, bool value) {
  writeVarintField(tag_num, value ? 1 : 0);
}

// Serialize a string field
void writeStringField(uint32_t tag_num, const char* value) {
  writeTag(tag_num, proto_util::kTagTypeLenDelimited);
  const uint32_t n = strlen(value);
  writeVarint(n);
  sendProtocolRawBytes(reinterpret_cast<const uint8_t*>(value), n);
}

// ---------- Downstream

//// 64 bits encoded as 7 bits per byte.
//static const int kMaxVarint64Bytes = 10;
//// 32 bits encoded as 7 bits per byte.
//static const int kMaxVarint32Bytes = 5;
//
//extern bool readRawByte(uint8_t* value) {
//  return esp8266::rx_fifo.getByte(value);
//}
//
//extern bool readVarint64(uint64_t* value) {
//  if (is_panic_mode) {
//    return false;
//  }
//  uint64_t result = 0;
//  for (int i = 0; i < kMaxVarint64Bytes; i++) {
//    uint8_t b;
//    if (!esp8266::rx_fifo.peekByte(i, &b)) {
//      return false;
//    }
//    result |= static_cast<uint64_t>(b & 0x7F) << (7 * i);
//    if (!(b & 0x80)) {
//      *value = result;
//      if (!esp8266::rx_fifo.skipBytes(i+1)) {
//        protocolPanic("varint64 skip");
//        return false;
//      }
//      debug.printf("varint64: %llu", *value);
//      return true;
//    }
//  }
//  protocolPanic("varint64 overrun");
//  return false;
//}
//
//extern bool readVarint32(uint32_t* value) {
//  if (is_panic_mode) {
//    return false;
//  }
//  int kaka = 0;
//  uint32_t result = 0;
//  for (int i = 0; i < kMaxVarint32Bytes; i++) {
//    uint8_t b;
//    if (!esp8266::rx_fifo.peekByte(i, &b)) {
//      return false;
//    }
//    //debug.printf("b=%02x\n", b);
//    result |= static_cast<uint32_t>(b & 0x7F) << (7 * i);
//    if (!(b & 0x80)) {
//      *value = result;
//      if (!esp8266::rx_fifo.skipBytes(i+1)) {
//        protocolPanic("varint32 skip");
//        return false;
//      }
//      debug.printf("varint32: %u\n", *value);
//      return true;
//    }
//    kaka++;
//  }
//  debug.printf("i = %d\n", kaka);
//  protocolPanic("varint32 overrun");
//  return false;
//}
//
//bool readFieldTag(uint32_t* tag_num, uint8_t* tag_type) {
//  // Tags are small, not bothering with 64 bit varint.
//  uint32_t value;
//  if (!readVarint32(&value)) {
//    return false;
//  }
//  *tag_num = value >> 3;
//  *tag_type = value & 0x7;
//  return true;
//}


// Sending of messages is done in two passes to keep the memory footprint small. The first
// is a dry run pass that just counts the number of bytes sent and the second is the actual
// message sending which is prefixed by the message size.

void sendProtocolVersion() {
  is_counting_only_mode = false;
  //setCountingOnlyMode(false);
  writeVarint(38);
}

//void sendLoginRequestAndFlush(uint64_t device_id, uint64_t auth_token) {
//  debug.printf("id=%08x:%08x\n", static_cast<uint32_t>(device_id >> 32),
//      static_cast<uint32_t>(device_id));
//  debug.printf("auth=%08x:%08x\n", static_cast<uint32_t>(auth_token >> 32),
//      static_cast<uint32_t>(auth_token));
//  uint32_t pass0_size = 0;
//  uint32_t pass1_size = 0;
//
//  protobuf::setCountingOnlyMode(true);
//
//  for (int pass = 0; pass < 2; pass++) {
//    const uint32_t pass_start_count = protobuf::totalBytesWritten();
//
//    protobuf::writeStringField(1, "");  // Not used. Required field.
//    protobuf::writeStringField(2, kDomain);  // domain
//    sniprintf(tmp_buffer, sizeof(tmp_buffer), "%llu", device_id);
//    protobuf::writeStringField(3, tmp_buffer);  // user
//    protobuf::writeStringField(4, tmp_buffer);  // resource
//    sniprintf(tmp_buffer, sizeof(tmp_buffer), "%llu", auth_token);
//    protobuf::writeStringField(5, tmp_buffer);  // auth token
//    sniprintf(tmp_buffer, sizeof(tmp_buffer), "android-%llx", device_id);
//    protobuf::writeStringField(6, tmp_buffer);  // device_id
//    protobuf::writeVarintField(16, 2);  // auth_service = ANDROID_ID
//
//    if (pass == 0) {
//      pass0_size = protobuf::totalBytesWritten() - pass_start_count;
//      protobuf::setCountingOnlyMode(false);
//      // Send message header
//      protobuf::writeVarint(2);  // message tag for LoginRequest.
//      protobuf::writeVarint(pass0_size);
//    } else {
//      // This else section is for verification only.
//      pass1_size = protobuf::totalBytesWritten() - pass_start_count;
//    }
//  }
//
//  protobuf::flushWrites();
//  debug.printf("LoginRequest sent, %u (%u) bytes\n", pass1_size, pass0_size);
//}


//void sendProtocolVersion() {
//  esp8266::rx_fifo.putByte(38);
//  //esp8266::rx_fifo.putByte(1);
//}

void sendLoginRequest(uint64_t device_id, uint64_t auth_token) {
  debug.printf("id=%08x:%08x\n", static_cast<uint32_t>(device_id >> 32),
      static_cast<uint32_t>(device_id));
  debug.printf("auth=%08x:%08x\n", static_cast<uint32_t>(auth_token >> 32),
      static_cast<uint32_t>(auth_token));

//
//  debug.printf("x0=%x\n", auth_token);
//  debug.printf("x1=%lx\n", auth_token);
//  debug.printf("x2=%llx\n", auth_token);
//
//  debug.printf("u0=%u\n", auth_token);
//  debug.printf("u1=%lu\n", auth_token);
//  debug.printf("u2=%llu\n", auth_token);

  uint32_t pass0_size = 0;
  uint32_t pass1_size = 0;

  //setCountingOnlyMode(true);
  is_counting_only_mode = true;

  for (int pass = 0; pass < 2; pass++) {
    const uint32_t pass_start_count = total_bytes_written;

    //protobuf::totalBytesWritten();


    writeStringField(1, "");  // Not used. Required field.
    writeStringField(2, kDomain);  // domain

    const char* t = uint64ToDecimal(device_id);
    //snprintf(tmp_buffer, sizeof(tmp_buffer), "%llu", device_id);
    writeStringField(3, t);  // user
    writeStringField(4, t);  // resource

    t = uint64ToDecimal(auth_token);
    //snprintf(tmp_buffer, sizeof(tmp_buffer), "%llu", auth_token);
    writeStringField(5, t);  // auth token

    t = uint64ToHex(device_id);
    snprintf(tmp_buffer, sizeof(t), "android-%s", t);
    writeStringField(6, t);  // device_id

    writeVarintField(16, 2);  // auth_service = ANDROID_ID

    if (pass == 0) {
      pass0_size = total_bytes_written - pass_start_count;
      //protobuf::setCountingOnlyMode(false);
      is_counting_only_mode = false;
      // Send message header
      writeVarint(2);  // message tag for LoginRequest.
      writeVarint(pass0_size);
    } else {
      // This else section is for verification only.
      pass1_size = total_bytes_written - pass_start_count;
    }
  }

  //protobuf::flushWrites();
  debug.printf("LoginRequest sent, %u (%u) bytes\n", pass1_size, pass0_size);
}

// ---------- Miscellaneous

void setup() {
  reset();
}

void loop() {
  // Nothing to do.
}

extern void reset() {
  //is_panic_mode = false;
  total_bytes_written = 0;
  is_counting_only_mode = false;
  //write_buffer_size = 0;
  esp8266::rx_fifo.reset();
}

//void protocolPanic(const char* short_message) {
//  debug.printf("Protocol panic: %s\n", short_message);
//  if (!is_panic_mode) {
//    esp8266::reconnect();
//    is_panic_mode = true;
//  }
//}



}  // protobuf
