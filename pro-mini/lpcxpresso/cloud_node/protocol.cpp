
#include "protocol.h"

#include "debug.h"
#include "protocol_util.h"
#include "protocol_tx.h"
#include "protocol_rx.h"
#include "esp8266.h"

namespace protocol {

//static DigitalOut panic_pin(P0_11);  //  @@@@@@ debug only

static bool is_panic_mode;

// Delegate to protocol_gp so both protocol_tx and protocol_rx can
// access the panic flag.
void protocolPanic(const char* short_message) {
  if (!is_panic_mode) {
    //panic_pin.write(1);
    debug.printf("Protocol panic: %s\n", short_message);
    esp8266::abortCurrentConnection();
    is_panic_mode = true;
    //panic_pin.write(0);
  }
}

bool isPanicMode() {
  return is_panic_mode;
}

void initialize() {
  protocol_tx::initialize();
  protocol_rx::initialize();
  resetForANewConnection();
}

void polling() {
  protocol_tx::polling();
  protocol_rx::polling();
}

void resetForANewConnection() {
  protocol_tx::resetForANewConnection();
  protocol_rx::resetForANewConnection();

  // Handshake counters.
  protocol_util::out_messages_counter = 0;
  protocol_util::in_messages_counter = 0;
}

void dumpInternalState() {
  debug.printf("protocol: msgs in=%d, out=%d, panic=%d\n",
      protocol_util::in_messages_counter, protocol_util::out_messages_counter, is_panic_mode);
  protocol_rx::dumpInternalState();
  protocol_tx::dumpInternalState();
}

}  // namespace protocol
