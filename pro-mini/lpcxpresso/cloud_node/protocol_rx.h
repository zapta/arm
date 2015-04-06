#ifndef PROTOCOL_RX_H
#define PROTOCOL_RX_H

#include "mbed.h"

namespace protocol_rx {


extern void initialize();
extern void polling();

// Prepare for a new connection.
extern void resetForANewConnection();

enum EventType {
  // It's safe to assume that 0 means none.
  EVENT_NONE = 0,
  EVENT_LOGIN_RESPONSE,
  EVENT_HEARTBEAK_ACK,
  EVENT_DATA_MESSAGE_STANZA,
};

// TODO: have the events in a C union to reduce memory footprint?
// Content is valid only when currentEvent() == EVENT_LOGIN_RESPONSE;

// Login Response event.
struct RxLoginResponseEvent {
  int32_t error_code;
};
extern RxLoginResponseEvent rx_login_response_event;

// Data Message Stanza event
struct RxDataMessageStanzaEvent {
  // A string extracted from the AppData.value field of the message.
  const char*  value;
};
extern RxDataMessageStanzaEvent rx_data_message_stanza_event;

// The current ready event, or EVENT_NONE (also 0) if none.
extern EventType currentEvent();

// Call to continue the parsing to the next event.
extern void eventDone();

// Called by protocol.cpp on protocol panic.
extern void onProtocolPanic();

// For debugging.
extern void dumpInternalState();

}  // namespace protocol_rx

#endif  // PROTOCOL_RX_H
