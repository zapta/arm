#include "mbed.h"
#include <string.h>
#include <ctype.h>
#include "esp8266.h"
#include "debug.h"
#include "protocol.h"
#include "protocol_tx.h"
#include "_config.h"
#include "u8g_mbed.h"

#include "protocol_util.h"  // remove this dependency

static DigitalOut led(P0_20, 0);

static Timer timer;

static Timer led_timer;

static Timer time_in_current_state;

// The U8G instance (a C struct).
static u8g_t u8g;

// Screen dimensions,
// x range = [0, kMaxX]
// y range = [0, kMaxY]
static const int kMaxX = 128 - 1;
static const int kMaxY = 64 - 1;


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

// TODO: call rx loop in the iterations here.
// TODO: faster SPI clock
void draw() {
   // Prepare. Could be move to setup().
   u8g_SetFont(&u8g, u8g_font_6x10);
   u8g_SetFontRefHeightExtendedText(&u8g);
   u8g_SetDefaultForegroundColor(&u8g);
   u8g_SetFontPosTop(&u8g);

   // U8G picture loop. See more details here:
   // https://code.google.com/p/u8glib/wiki/tpictureloop
   u8g_FirstPage(&u8g);
    do {
      u8g_DrawStr(&u8g, kMaxX/2-32, 0, "CLOUD NODE");
      //u8g_DrawLine(&u8g, m.x.value, 0, m.x.value, kMaxY);
      //u8g_DrawLine(&u8g, 0, m.y.value, kMaxX, m.y.value);

      char bfr[15];
      snprintf(bfr, sizeof(bfr), "in: %lu", protocol_util::in_messages_counter);
      u8g_DrawStr(&u8g, 10, kMaxY-20, bfr);
      snprintf(bfr, sizeof(bfr), "out: %lu", protocol_util::out_messages_counter);
         u8g_DrawStr(&u8g, 10, kMaxY-9, bfr);
    } while (u8g_NextPage(&u8g));
 }


static void initialize() {
  timer.start();
  led_timer.start();
  heatbeat_timer.start();
  time_in_current_state.start();
  //state = NOT_CONNECTED;
  esp8266::initialize();
  protocol::initialize();
  // u8g initialization for the ssd1306 128x64 oled we use with SPI0.
  u8g_InitComFn(&u8g, &u8g_dev_ssd1306_128x64_hw_spi, u8g_com_hw_spi_fn);
  draw();

}


static void protocolPanic(const char* short_message) {
  protocol::protocolPanic(short_message);
  setState(ERROR);
  //state = ERROR;
}

// Called periodic to dump state for debugging.
static void dumpInternalState() {
  esp8266::dumpInternalState();
  protocol::dumpInternalState();
  debug.printf("main: s=%d t=%d\n", state,
      time_in_current_state.read_ms() / 1000);
}

// Handler for the CONNECTED state.
static void polling_connectedState() {
  // TODO: define a const for the heartbeat interval.
  if (heatbeat_timer.read_ms() > 3 * 60 * 1000) {
    heatbeat_timer.reset();
    protocol_tx::sendHeartbeatPing();
    return;
  }

  const protocol_rx::EventType event_type = protocol_rx::currentEvent();

  if (!event_type) {
    return;
  }

  // Handle for the specific event derived from selected incoming
  // MCS message types.
  switch (event_type) {
    case protocol_rx::EVENT_DATA_MESSAGE_STANZA: {
      const protocol_rx::RxDataMessageStanzaEvent& event = protocol_rx::rx_data_message_stanza_event;
      debug.printf("*** main: message stanza event: [%s]=[%s]\n", event.key, event.value);
      break;
    }

    // All other events
    default:
      debug.printf("Unexpected event type: %d\n", event_type);
  }
  protocol_rx::eventDone();
}

static void polling() {
  esp8266::polling();
  protocol::polling();

  led.write(led_timer.read_ms() < 200);
  if (led_timer.read_ms() >= 3000) {
    led_timer.reset();
    dumpInternalState();
    draw();
  }

  const uint32_t connection_id = esp8266::connectionId();
  // Handle connection change.
  if (connection_id != last_connection_id) {
    last_connection_id = connection_id;
    protocol::resetForANewConnection();
    setState(NOT_CONNECTED);
    return;
  }

  switch (state) {
    case NOT_CONNECTED:
      // If we got here, connection_id is non zero and is equals to last_connection_id.
      if (connection_id) {
        debug.printf("## SENDING LOGIN\n");
        //protocol_tx::sendProtocolVersion();
        protocol_tx::sendProtocolVersionAndLoginRequest(config::device_id, config::auth_token);
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
      break;
    }

    case CONNECTED:
      polling_connectedState();
      break;

    case ERROR:
      // This state exists by the connection change logic at the
      // top of this function.
      break;

    default:
      debug.printf("Unknown: %d\n", state);
      setState(ERROR);
  }
}

// One time initialization and then continuous polling.
int main(void) {
  initialize();
  for (;;) {
    polling();
  }
}
