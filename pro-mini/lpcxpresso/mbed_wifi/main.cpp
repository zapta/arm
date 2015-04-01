#include "mbed.h"
#include <string.h>
#include <ctype.h>
#include "esp8266.h"
#include "debug.h"
#include "protocol.h"
#include "_config.h"

// expose Panic in protocol.h and remove this.
#include "protocol_util.h"

static DigitalOut led(P0_20, 0);

static Timer timer;

static Timer led_timer;

enum MainState {
  NOT_CONNECTED,
  WAIT_LOGIN_RESPONSE,
  CONNECTED,
  CONNECTION_END,
  ERROR,
};

static MainState state = NOT_CONNECTED;
static uint32_t last_connection_id = -1;

static void setup() {
  timer.start();
  led_timer.start();
  esp8266::setup();
  protocol::setup();
}

static void loop() {
  esp8266::loop();
  protocol::loop();

  led.write(led_timer.read_ms() < 200);
  if (led_timer.read_ms() >= 3000) {
    led_timer.reset();
    esp8266::dumpState();
    debug.printf("main: %d\n", state);
  }

  const uint32_t connection_id = esp8266::connectionId();
    // Handle connection change.
    if (connection_id != last_connection_id) {
      last_connection_id = connection_id;
      protocol_rx::start();
      state = NOT_CONNECTED;
      return;
    }

  switch (state) {
    case NOT_CONNECTED:
      // If we got here, connection_id is non zero and is equals to last_connection_id.
      if (connection_id) {
        debug.printf("## SENDING LOGIN\n");
        protocol_tx::sendProtocolVersion();
        protocol_tx::sendLoginRequest(config::device_id, config::auth_token);
        state = WAIT_LOGIN_RESPONSE;
      }
      break;

    case WAIT_LOGIN_RESPONSE: {
      protocol_rx::EventType event_type = protocol_rx::currentEvent();
      if (!event_type) {
        return;
      }
      protocol_rx::eventDone();
      if (event_type == protocol_rx::EVENT_LOGIN_RESPONSE) {
        state = CONNECTED;
      } else {
        protocol_util::protocolPanic("NO LOGIN");
        state = ERROR;
      }
    }
      break;

    case CONNECTED:
      // TODO: handle other events here
      break;

    case ERROR:
      // Exit by the connection change logic at the top of this function.
      break;

    default:
      debug.printf("Unknown: %d\n", state);
      state = ERROR;
  }
}

int main(void) {
  setup();
  for (;;) {
    loop();
  }
}
