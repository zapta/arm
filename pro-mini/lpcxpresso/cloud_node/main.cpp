#include <ctype.h>
#include <string.h>

#include "_config.h"
#include "debug.h"
#include "esp8266.h"
#include "mbed_u8g.h"
#include "mbed.h"
#include "protocol_tx.h"
#include "protocol.h"
#include "system_time.h"
#include "passive_timer.h"

#include "protocol_util.h"  // remove this dependency

// TODO: remove dependency, for debugging
#include "esp8266.h"


// The ARM PRO MINI LED output.
static DigitalOut led(P0_20, 0);

// Display update interval.
static const int kDisplayIntervalMillis = 1000;
static PassiveTimer display_interval_millis;

// Text message to display. Null terminated string.
static char text_message[20] = {
    0 };

static PassiveTimer time_since_last_text_message;

// Timer for scheduling outgoing heartbeat messages.
// TODO: move this to the protocol to determine when a heatbeat should be sent.
static PassiveTimer heatbeat_timer;

// Time since last state change.
static PassiveTimer time_in_current_state;

// The last connection id reported by the esp8266 module. Zero when no
// connection and keep incrementing on each new connection.
static uint32_t last_connection_id = -1;

// The graphic library instance (a C struct).
static u8g_t u8g;

// Screen dimensions,
//static const int kMaxX = 128 - 1;
//static const int kMaxY = 64 - 1;

enum State {
  NOT_CONNECTED,
  WAITING_FOR_LOGIN_RESPONSE,
  CONNECTED,
  CONNECTION_END,
  ERROR,
};

static State state = NOT_CONNECTED;

// Return display name for given state.
static const char* stateName(State state) {
  switch (state) {
    case NOT_CONNECTED:
      return "DISCONNECTED";
    case WAITING_FOR_LOGIN_RESPONSE:
      return "LOGIN";
    case CONNECTED:
      return "CONNECTED";
    case CONNECTION_END:
      return "CONN END";
    case ERROR:
      return "ERROR";
    default:
      return "???";
  }
}

static void setState(State new_state) {
  debug.printf("main %s -> %s\n", stateName(state), stateName(new_state));
  state = new_state;
  time_in_current_state.reset();
}

// TODO: call rx loop in the iterations here.
// TODO: faster SPI clock
void drawDisplay() {
  debug.printf("**REDRAW**\n");

  // Prepare. Could be move to setup().
  u8g_SetFont(&u8g, u8g_font_6x10);
  u8g_SetFontRefHeightExtendedText(&u8g);
  //u8g_SetDefaultBackgroundColor(&u8g);
  u8g_SetDefaultForegroundColor(&u8g);
  u8g_SetFontPosTop(&u8g);

  //const int t0 = time_in_current_state.millis();
  const int t1 = time_in_current_state.secs();
  const int ss = t1 % 60;
  const int t2 = t1 / 60;
  const int mm = t2 % 60;
  const int hh = t2 / 60;
//  debug.printf("clock: t1:%d, t2:%d, ss:%d, mm:%d, hh:%d\n", t1, t2,
//      ss, mm, hh);

  // 1 sec per pixel. The u8g library will do the clipping to screen width.
  const uint32_t kMaxBarSecs = 5 * 60;
  const uint32_t kMinBarPixels = 2;
  const uint32_t kMaxBarPixles = 128; // screen width
  const uint32_t test_message_secs = time_since_last_text_message.secs();
  int bar_length_in_pixels =  kMinBarPixels + ((test_message_secs >= kMaxBarSecs)
      ? kMaxBarPixles - kMinBarPixels
      : (test_message_secs * (kMaxBarPixles - kMinBarPixels)) / kMaxBarSecs);

  // U8G picture loop. See more details here:
  // https://code.google.com/p/u8glib/wiki/tpictureloop
  u8g_FirstPage(&u8g);
  do {
    // EXPERIMENTAL
    // TODO: clean this.
    esp8266::polling();
    protocol::polling();

    //u8g_DrawStr(&u8g, kMaxX / 2 - 32, 0, "CLOUD NODE");
    u8g_DrawStr(&u8g, 31, 0, "Cloud Thing");


    // TODO: determine size, make this buffer shared.
    static char bfr[20];

    if (state == CONNECTED || state == WAITING_FOR_LOGIN_RESPONSE) {
      snprintf(bfr, sizeof(bfr), "%s [%lu]%s%s", stateName(state),
          last_connection_id, esp8266::had_bad_line? "*" : "",
          protocol::isPanicMode()? "!" : "");
      u8g_DrawStr(&u8g, 0, 38, bfr);
    } else {
      u8g_DrawStr(&u8g, 0, 38, stateName(state));
    }

    if (state == CONNECTED) {
      // TODO: temp hack for the initial 'true' message. Filter out
      // using the AppData key. For now we use here a hard coded filter.
      if (strcmp(text_message, "true") != 0) {
        u8g_DrawStr(&u8g, 4, 19, text_message);
      }

      u8g_DrawFrame(&u8g, 0, 16, 128, 15);

      // Draw time-since-last update bar.
      u8g_DrawHLine(&u8g, 0, 63, bar_length_in_pixels);
    }

    snprintf(bfr, sizeof(bfr), "I:%lu O:%lu %d.%02d.%02d",
        protocol_util::in_messages_counter, protocol_util::out_messages_counter,
        hh, mm, ss);
    u8g_DrawStr(&u8g, 0, 50, bfr);
  } while (u8g_NextPage(&u8g));
}

static void initialize() {
  system_time::initialize();
  //display_interval_millis.start();
  //heatbeat_timer.start();
  //time_in_current_state.start();
  //time_since_last_text_message.start();
  esp8266::initialize();
  protocol::initialize();
  // u8g initialization for the ssd1306 128x64 oled we use with SPI0.
  u8g_InitComFn(&u8g, &u8g_dev_ssd1306_128x64_hw_spi, u8g_com_hw_spi_fn);
  drawDisplay();
}

static void protocolPanic(const char* short_message) {
  protocol::protocolPanic(short_message);
  setState(ERROR);
}

// Called periodic to dump state for debugging.
static void dumpInternalState() {
  esp8266::dumpInternalState();
  protocol::dumpInternalState();
  debug.printf("main: s=%s id=%d t=%d\n", stateName(state), last_connection_id,
      time_in_current_state.secs());
}

// Handler for the CONNECTED state.
static void polling_connectedState() {
  // TODO: define a const for the heartbeat interval.
  if (heatbeat_timer.millis() > 3 * 60 * 1000) {
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
      const protocol_rx::RxDataMessageStanzaEvent& event =
          protocol_rx::rx_data_message_stanza_event;
      text_message[0] = '\0';
      strncpy(text_message, event.value, sizeof(text_message) - 1);
      time_since_last_text_message.reset();
      break;
    }

      // All other events
    default:
      debug.printf("Unexpected event type: %d\n", event_type);
  }
  protocol_rx::eventDone();
}

// Main poling function. Called repeatedly.
static void polling() {
  system_time::polling();
  esp8266::polling();
  protocol::polling();

  {
    const uint32_t displal_interval_time_millis = display_interval_millis.millis();
  led.write(displal_interval_time_millis < 200);
  if (displal_interval_time_millis >= kDisplayIntervalMillis) {
    display_interval_millis.reset();
    dumpInternalState();
    drawDisplay();
    return;
  }
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
        protocol_tx::sendProtocolVersionAndLoginRequest(config::device_id,
            config::auth_token);
        setState(WAITING_FOR_LOGIN_RESPONSE);
      }
      break;

    case WAITING_FOR_LOGIN_RESPONSE: {
      const protocol_rx::EventType event_type = protocol_rx::currentEvent();
      if (event_type) {
        if (event_type == protocol_rx::EVENT_LOGIN_RESPONSE
            && !protocol_rx::rx_login_response_event.error_code) {
          protocol_rx::eventDone();
          setState(CONNECTED);
          time_since_last_text_message.reset();
          heatbeat_timer.reset();
        } else
          protocolPanic("BAD LOGIN");
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
