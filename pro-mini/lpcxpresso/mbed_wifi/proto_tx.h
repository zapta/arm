// Light weight on-the-fly de/serializaiton of protocol buffers primities required
// for the protocol.
//
// Writes are sent to the esp8266 connection and reads are received from that
// connection.

#ifndef PROTO_TX_H
#define PROTO_TX_H

#include "mbed.h"

namespace proto_tx {

//// Tag type. Represents the encoding type of the value that follows it.
//static const uint8_t kTagTypeVarint = 0;
//static const uint8_t kTagTypeFixed64 = 1;
//static const uint8_t kTagTypeLenDelimited = 2;
//static const uint8_t kTagTypeStartGroup = 3;
//static const uint8_t kTagTypeEndGroup = 4;
//static const uint8_t kTagTypeFixed32 = 5;
//
//// ---------- TX
//
//extern void writeVarint(uint64_t value);
//extern void writeVarintField(uint32_t tag_num, uint64_t value);
//extern void writeBoolField(uint32_t tag_num, bool value);
//extern void writeStringField(uint32_t tag_num, const char* value);
//
//extern uint32_t totalBytesWritten();
//extern bool setCountingOnlyMode(bool new_value);
//
////extern void flushWrites();
//
//
//// ---------- RX
//
//// TODO: delete the RX section, it is moved to the parser
//
//// If a byte is available in the input stream consume it and return true and
//// its value. Otherwise return false.
//extern bool readRawByte(uint8_t* value);
//
//// We expect a varint to be next in the input stream.
//// If a complete varint is found return true and its value.
//// If input is empty or the varint is incomplete yet return false.
//// In case of an error close the connection.
//extern bool readVarint64(uint64_t* value);
//
//// Same but assumes a smaller value that fits in 32 bit.
//extern bool readVarint32(uint32_t* value);
//
//
//// We expect a field tag to be next in the input stream.
//// If a complete tag is found return true and it's num and type.
//// If the input is empty or the tag is incomplete yet return false.
//// In case of an error close the connection.
//extern bool readFieldTag(uint32_t* tag_num, uint8_t* tag_type);
//


extern void sendProtocolVersion();

extern void sendLoginRequest(uint64_t device_id, uint64_t auth_token);

// One time initialization.
extern void setup();
extern void loop();
// Polling from main loop.

// Does nothing for now.
//inline void loop ();

// Cleanup for a new connection.
extern void reset();

// Call this when the protocol encounters an unrecoverable error. This will
// cause the network connection to drop and reconnect.
//extern void protocolPanic(const char* short_message);

}  // proto_tx

#endif  // PROTOBUF_H
