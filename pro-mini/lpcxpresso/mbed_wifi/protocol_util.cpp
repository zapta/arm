
#include "protocol_util.h"

#include "debug.h"
#include "esp8266.h"

namespace protocol_util {

// Exported by protocol_util.h
bool is_panic_mode = false;

void protocolPanic(const char* short_message) {
  if (!is_panic_mode) {
    debug.printf("Protocol panic: %s\n", short_message);
    esp8266::reconnect();
    is_panic_mode = true;
  }
}

}  // namespace protocol_util
