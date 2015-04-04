// Resources:
// https://github.com/google/protobuf/blob/master/src/google/protobuf/io/coded_stream.cc
// https://github.com/crosswalk-project/chromium-crosswalk/blob/master/google_apis/gcm/protocol/mcs.proto
// https://developers.google.com/protocol-buffers/docs/encoding
// http://en.wikipedia.org/wiki/Protocol_Buffers
// https://github.com/ChromiumWebApps/chromium/tree/master/google_apis/gcm/base

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "mbed.h"
#include "protocol_tx.h"
#include "protocol_rx.h"

namespace protocol {

// One time initialization..
extern void setup();

// Reset protocol for a new connection.
extern void resetForANewConnection();

// Continuous polling.
extern void loop();

// Call this on protcol errors. It propogates the panic call to the
// protcol rx and tx modules and triggers a connection restart.
extern void protocolPanic(const char* short_message);

extern bool isPanicMode();

// For debugging.
extern void dumpInternalState();

}  // namespace protocol

#endif  // PROTOCOL_H
