#ifndef PROTOCOL_TX_H
#define PROTOCOL_TX_H

#include "mbed.h"

namespace protocol_tx {

// The first thing that is sent on a new connection.
extern void sendProtocolVersion();
// Sent once on each new connection immedielty after the protocol version.
extern void sendLoginRequest(uint64_t device_id, uint64_t auth_token);
// Send a heatbeat pin. Local and acked message ids are filled in automatically.
extern void sendHeartbeatPing();

// One time initialization.
extern void setup();

// Continious polling.
extern void loop();

// Cleanup for a new connection.
extern void reset();
}  // namespace protocol_tx

#endif  // PROTOCOL_H
