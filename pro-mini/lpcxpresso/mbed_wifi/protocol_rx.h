#ifndef PROTOCOL_RX_H
#define PROTOCOL_RX_H

#include "mbed.h"

namespace protocol_rx {


extern void setup();
extern void loop();

// Start/stop the downstream data. We restart it for each new connection.
extern void stop();
extern void start();

enum EventType {
  // It's safe to assume that 0 means none.
  EVENT_NONE = 0,
  EVENT_LOGIN_RESPONSE
};

// TODO: have the events in a C union to reduce memory footprint?
struct LoginResponseEvent {
  uint32_t id;
  uint64_t secret;
};
extern LoginResponseEvent login_response_event;

// Events generating from parsing incoming messages.
extern EventType currentEvent();
extern void eventDone();

// For debugging.
extern void dumpInternalState();

}  // namespace protocol_rx

#endif  // PROTOCOL_RX_H
