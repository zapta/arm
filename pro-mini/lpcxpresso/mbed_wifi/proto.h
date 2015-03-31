#ifndef PROTO_H
#define PROTO_H

#include "mbed.h"
#include "proto_tx.h"
#include "proto_rx.h"

namespace proto {

// -------- TX

//extern void sendProtocolVersion();
//extern void sendLoginRequest(uint64_t device_id, uint64_t auth_token);

// -------- RX

//enum EventType {
//  // It's safe to assume that 0 means none.
//  EVENT_NONE = 0,
//  //EVENT_PROTOCOL_VERSION,
//  EVENT_LOGIN_RESPONSE
//};
//
//struct ProtocolVersionEvent {
//  uint32_t version;
//};
//
//struct LoginResponseEvent {
//  uint32_t id;
//  uint64_t secret;
//};
//
//// TODO: make this const.
////extern ProtocolVersionEvent event_protocol_version;
////extern const ProtocolVersionEvent& protocol_version_event;
//extern LoginResponseEvent event_login_response;
//
//extern EventType currentEvent();
//extern void nextEvent();


// -------- MISC

// Initialization.
extern void setup();

// Reset protocol for a new connection.
extern void reset();

extern void loop();

extern void dumpState();

}  // namespace proto

#endif  // PROTO_H
