#include "mbed.h"
#include <string.h>
#include <ctype.h>
#include "esp8266.h"
#include "debug.h"
#include "protocol.h"
#include "protocol_tx.h"
#include "_config.h"

// expose Panic in protocol.h and remove this.
#include "protocol_util.h"

static DigitalOut led(P0_20, 0);

static Timer timer;

static Timer led_timer;

static Timer time_in_current_state;

// TODO: move this to the protocol to determine when a heatbeat should be
// sent.
static Timer heatbeat_timer;

enum MainState {
  NOT_CONNECTED,
  WAIT_LOGIN_RESPONSE,
  CONNECTED,
  CONNECTION_END,
  ERROR,
};

static MainState state = NOT_CONNECTED;

static uint32_t last_connection_id = -1;

static void setState(MainState new_state) {
  debug.printf("main %d -> %d\n", state, new_state);
  state = new_state;
  time_in_current_state.reset();
}

static void setup() {
  timer.start();
  led_timer.start();
  heatbeat_timer.start();
  time_in_current_state.start();
  //state = NOT_CONNECTED;
  esp8266::setup();
  protocol::setup();
}

static void protocolPanic(const char* short_message) {
  protocol_util::protocolPanic(short_message);
  setState(ERROR);
  //state = ERROR;
}

static void loop() {
  esp8266::loop();
  protocol::loop();

  led.write(led_timer.read_ms() < 200);
  if (led_timer.read_ms() >= 3000) {
    led_timer.reset();
    esp8266::dumpState();
    debug.printf("main: s=%d t=%d\n", state,
        time_in_current_state.read_ms() / 1000);
  }

  const uint32_t connection_id = esp8266::connectionId();
  // Handle connection change.
  if (connection_id != last_connection_id) {
    last_connection_id = connection_id;
    protocol::reset();
    setState(NOT_CONNECTED);
    //state = NOT_CONNECTED;
    return;
  }

  switch (state) {
    case NOT_CONNECTED:
      // If we got here, connection_id is non zero and is equals to last_connection_id.
      if (connection_id) {
        debug.printf("## SENDING LOGIN\n");
        protocol_tx::sendProtocolVersion();
        protocol_tx::sendLoginRequest(config::device_id, config::auth_token);
        setState(WAIT_LOGIN_RESPONSE);
      }
      break;

    case WAIT_LOGIN_RESPONSE: {
      const protocol_rx::EventType event_type = protocol_rx::currentEvent();
      if (!event_type) {
        return;
      }
      // Here we have an event. We expect a login response with no error code.
      if (event_type != protocol_rx::EVENT_LOGIN_RESPONSE
          || protocol_rx::rx_login_response_event.error_code) {
        protocolPanic("BAD LOGIN");
      } else {
        protocol_rx::eventDone();
        setState(CONNECTED);
        heatbeat_timer.reset();
      }
    }
      break;

    case CONNECTED: {
      const protocol_rx::EventType event_type = protocol_rx::currentEvent();
      if (event_type) {
        switch (event_type) {
          case protocol_rx::EVENT_HEARTBEAK_ACK:
            debug.printf("*** main: heartbet ACK: %d, %d\n",
                protocol_rx::rx_heartbeat_ack_event.stream_id,
                protocol_rx::rx_heartbeat_ack_event.last_stream_id_received);
            break;
          default:
            debug.printf("Unexpected event type: %d\n", event_type);
        }
        protocol_rx::eventDone();
      }

      if (heatbeat_timer.read_ms() > 15000) {
        heatbeat_timer.reset();
        protocol_tx::sendHeartbeatPing();
        //debug.printf("main: connected %d seconds\n", time_in_current_state.read_ms()/1000);
      }
      // TODO: handle other events here
    }
      break;

    case ERROR:
      // Exit by the connection change logic at the top of this function.
      break;

    default:
      debug.printf("Unknown: %d\n", state);
      setState(ERROR);
      //state = ERROR;
  }
}

int main(void) {
  setup();
  for (;;) {
    loop();
  }
}
