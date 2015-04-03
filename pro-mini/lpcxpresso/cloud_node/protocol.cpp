
#include "protocol.h"

#include "debug.h"
#include "protocol_util.h"
#include "protocol_tx.h"
#include "protocol_rx.h"
#include "esp8266.h"
//#include "inttypes.h"

namespace protocol {

static bool is_panic_mode;

// Delegate to protocol_gp so both protocol_tx and protocol_rx can
// access the panic flag.
void protocolPanic(const char* short_message) {
  if (!is_panic_mode) {
    debug.printf("Protocol panic: %s\n", short_message);
    esp8266::abortCurrentConnection();
    is_panic_mode = true;
  }
}

bool isPanicMode() {
  return is_panic_mode;
}


void setup() {
  protocol_tx::setup();
  protocol_rx::setup();
  is_panic_mode = false;
}

void loop() {
  protocol_tx::loop();
  protocol_rx::loop();
}

void resetForANewConnection() {
  protocol_tx::resetForANewConnection();
  protocol_rx::resetForANewConnection();
  is_panic_mode = false;
}

void dumpInternalState() {
  protocol_rx::dumpInternalState();
  protocol_tx::dumpInternalState();
}

}  // namespace protocol
