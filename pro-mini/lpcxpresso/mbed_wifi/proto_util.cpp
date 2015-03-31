// Implementation of protobuff.h.
//
// Resources:
// https://github.com/google/protobuf/blob/master/src/google/protobuf/io/coded_stream.cc
// https://github.com/crosswalk-project/chromium-crosswalk/blob/master/google_apis/gcm/protocol/mcs.proto
// https://developers.google.com/protocol-buffers/docs/encoding
// http://en.wikipedia.org/wiki/Protocol_Buffers
// https://github.com/ChromiumWebApps/chromium/tree/master/google_apis/gcm/base

#include "proto_util.h"
#include "debug.h"
#include "esp8266.h"

namespace proto_util {


// ---------- Upstream

bool is_panic_mode = false;

//void isPanic() {
//  return is_panic_mode;
//}
// Total bytes written so far.
//static uint32_t total_bytes_written = 0;
//
//// When true, we don't actually send out data, just incrementing total_bytes_written.
//// Useful to determine message size before acutally sending it in a second pass.
//static bool is_counting_only_mode = false;

//bool setCountingOnlyMode(bool new_value) {
//  bool old_value = is_counting_only_mode;
//  is_counting_only_mode = new_value;
//  return old_value;
//}
//
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
//void writeRawByte(uint8_t b) {
//  if (!is_counting_only_mode) {
//    //if (write_buffer_size >= kWriteBufferMaxSize) {
//    //  flushWrites();
//    //}
//    esp8266::tx_fifo.putByte(b);
//    //write_buffer[write_buffer_size++] = b;
//    debug.printf("TX[%02x]\n", b);
//  }
//  total_bytes_written++;
//}


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
//void sendProtocolRawBytes(const uint8_t* data, uint32_t size) {
//  while (size--) {
//    writeRawByte(*data++);
//  }
//}

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
//void writeVarint(uint64_t value) {
//  while (value > 0x7F) {
//    writeRawByte((static_cast<uint8_t>(value) & 0x7F) | 0x80);
//    value >>= 7;
//  }
//  writeRawByte(static_cast<uint8_t>(value) & 0x7F);
//}

// Serialize a field tag.
//void writeTag(uint32_t tag_num, uint8_t tag_type) {
//  writeVarint(tag_num << 3 | tag_type);
//}
//
//// Serialize a varint field of given tag. Can be used for
//// ints and enums.
//void writeVarintField(uint32_t tag_num, uint64_t value) {
//  writeTag(tag_num, kTagTypeVarint);
//  writeVarint(value);
//}
//
//// Serialize a bool field of given tag.
//void writeBoolField(uint32_t tag_num, bool value) {
//  writeVarintField(tag_num, value ? 1 : 0);
//}
//
//// Serialize a string field
//void writeStringField(uint32_t tag_num, const char* value) {
//  writeTag(tag_num, kTagTypeLenDelimited);
//  const uint32_t n = strlen(value);
//  writeVarint(n);
//  sendProtocolRawBytes(reinterpret_cast<const uint8_t*>(value), n);
//}
//
//// ---------- Downstream
//
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
//
//
//// ---------- Miscellaneous
//
//void setup() {
//  reset();
//}
//
//extern void reset() {
//  is_panic_mode = false;
//  total_bytes_written = 0;
//  is_counting_only_mode = false;
//  //write_buffer_size = 0;
//  esp8266::rx_fifo.reset();
//}

void protocolPanic(const char* short_message) {
  debug.printf("Protocol panic: %s\n", short_message);
  if (!is_panic_mode) {
    esp8266::reconnect();
    is_panic_mode = true;
  }
}



}  // protobuf
