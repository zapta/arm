
#include "protocol.h"

#include "debug.h"
#include "protocol_util.h"
#include "protocol_tx.h"
#include "protocol_rx.h"
#include "esp8266.h"
//#include "inttypes.h"

namespace protocol {

void setup() {
  protocol_tx::setup();
  protocol_rx::setup();
  protocol_util::is_panic_mode = false;
}

void loop() {
  protocol_tx::loop();
  protocol_rx::loop();
}

void resetForANewConnection() {
  protocol_tx::resetForANewConnection();
  protocol_rx::resetForANewConnection();
  protocol_util::is_panic_mode = false;
}

void dumpInternalState() {
  protocol_rx::dumpInternalState();
  protocol_tx::dumpInternalState();
}

}  // namespace protocol
