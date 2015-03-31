#ifndef PROTO_RX_H
#define PROTO_RX_H

#include "mbed.h"

namespace proto_rx {

class Listener {
public:
//  // Each variable length field is handled according to one of these policies.
//  enum FieldPolicy {
//    VAR_LEN_SKIP,
//    VAR_LEN_DATA,
//    VAR_LEN_SUB_MESSAGE,
//  };

  // Message stack events. Called for each top message (level 0) and
  // sub messages (that were resolved as VAR_LEN_SUB_MESSAGE).
  virtual void onMessageEnter(uint8_t new_nesting_level, uint8_t tag_num) = 0;
  virtual void onMessageExit(uint8_t new_nestingLevel) = 0;

  // Called after a varint field was parsed.
  virtual void onVarintField(uint8_t tag_num, uint64_t value) = 0;

  // Called before parsing the content of a variable length data field.
  // Listener should resolve how to handle this field.
  virtual bool isSubMessage(uint8_t tag_num, uint32_t length) = 0;

  // Called for variable length fields that were resolved as VAR_LEN_DATA.
  virtual void onDataFieldStart(uint8_t tag_num) = 0;
  virtual void onDataFieldByte(uint8_t tag_num, uint8_t byte_value) = 0;
  virtual void onDataFieldEnd(uint8_t tag_num) = 0;
};

extern void setup();
extern void loop();
extern void stop();
extern void start();

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

//extern void setup();
////extern void loop();
//extern void reset();
//
//extern void dumpState();

}  // namespace proto_rx

#endif  // PROTO_RX_H
