#ifndef PROTOCOL_TX_H
#define PROTOCOL_TX_H

#include "mbed.h"

namespace protocol_tx {

// Is it ok to send a message.
extern bool isReadyToSend();

// Should be sent once on a new connection.
extern void sendProtocolVersionAndLoginRequest(uint64_t device_id, uint64_t auth_token);

// Send a heatbeat pin. Local and acked message ids are filled in automatically.
extern void sendHeartbeatPing();

// One time initialization.
extern void setup();

// Continious polling.
extern void loop();

// Cleanup for a new connection.
extern void resetForANewConnection();

// For debugging.
extern void dumpInternalState();

}  // namespace protocol_tx

#endif  // PROTOCOL_H
