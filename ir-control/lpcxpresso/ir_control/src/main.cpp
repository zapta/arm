// TV/AV master/slave IR controller.
//
// This program aim to have the AV system at the same on/off state
// as the TV. It senses the on/off states of both the AV and the TV and
// if different, it issues IR commands to change the state of the AV
// to that of the TV.
//
// This allows the on/off button on the TV remote control to control
// also the AV system.
//
// The on/off statuses of the AV and TV are senses via isolated USB
// connectors that sense the +5V from the devices. The AV IR control
// is currently hard coded to my Sony AV system but can be changed
// by modifying ir_tx.cpp.

#include "ir_tx.h"
#include "common.h"
#include "sense.h"

// Timeouts for waiting for AV response to IR commands.
const static int kWaitForAvOnTimeoutMillis = 3000;
const static int kWaitForAvOffTimeoutMillis = 3000;

// Status LED is at GPIO0_20.
static DigitalOut led(P0_20, 0);

// Timer for led blinking.
static Timer led_timer;

// Timer for dumping status to serial port (for debugging).
static Timer status_dump_timer;

// Timer for control state machine timeouts. Reset on each
// state change.
// May overflow in AV_EQUALS_TV since it's not time bounded.
static Timer time_in_state;

enum State {
  BOOT,
  DISPATCH,
  AV_EQUALS_TV,
  WAIT_AV_ON,
  WAIT_AV_OFF,
};

static State state = BOOT;

static void change_state(State new_state) {
  PRINTF("State %d -> %d\r\n", state, new_state);
  time_in_state.reset();
  state = new_state;
}

static void setup() {
  led_timer.start();
  status_dump_timer.start();
  time_in_state.start();

  ir_tx::setup();
  sense::setup();
}

// Led control.
static void loop_led_heartbeat() {
  // Set LED state.
  const int led_timer_ms = led_timer.read_ms();
  const bool is_av_equals_tv = (sense::is_av_on() == sense::is_tv_on());
  led = ir_tx::tx_packets_pending() || (led_timer_ms <= 100);

  // Faster blink when AV != TV.
  const int blink_period_ms = is_av_equals_tv ? 3000 : 400;
  if (led_timer_ms >= blink_period_ms) {
    led_timer.reset();
  }
}

// Periodic status dump to serial port.
static void loop_status_dump() {
  if (status_dump_timer.read_ms() > 2000) {
    status_dump_timer.reset();
    ir_tx::dump_state();
    PRINTF("TV=%u, AV=%u, packets=%d, state=%u (t="
        "%d)\r\n", sense::is_tv_on(), sense::is_av_on(),
        ir_tx::tx_packets_pending(), state,
        (state == AV_EQUALS_TV) ? -1 : time_in_state.read_ms());
  }
}

// Control state machine handling.
static void loop_state() {
  // Cache common values.
  const int ms_in_state = time_in_state.read_ms();
  const bool av_on = sense::is_av_on();
  const bool tv_on = sense::is_tv_on();

  switch (state) {

  // Initial delay to let the sense input debouncers stabilize.
  case BOOT:
    if (ms_in_state >= 1000) {
      change_state(DISPATCH);
    }
    break;

    // Compare the AV and TV state and dispatch state.
  case DISPATCH:
    if (av_on == tv_on) {
      change_state(AV_EQUALS_TV);
      return;
    }

    // Wait until previous tx completed before we can issue a new command.
    if (ir_tx::tx_packets_pending()) {
      return;
    }

    // Start transmitting an IR AV toggle command.
    ir_tx::start_tx(3);
    change_state(av_on ? WAIT_AV_OFF : WAIT_AV_ON);
    return;

    // The stable state where both AV and TV are on or off.
    // Note: do not use here time in state since it can overflow.
  case AV_EQUALS_TV:
    if (av_on != tv_on) {
      change_state(DISPATCH);
    }
    return;

    // An IR command was sent to turn the AV on. Wait for for AV on or timeout.
  case WAIT_AV_ON:
    if (av_on) {
      PRINTF("ON after %d ms\r\n", ms_in_state)
      change_state(DISPATCH);
      return;
    }
    // TODO: set a const for av on timeout
    if (ms_in_state >= kWaitForAvOnTimeoutMillis) {
      PRINTF("ON Timeout\r\n")
      change_state(DISPATCH);
    }
    return;

    // An IR command was sent to turn the AV off. Wait for for AV off or timeout.
  case WAIT_AV_OFF:
    if (!av_on) {
      PRINTF("OFF after %d ms\r\n", ms_in_state)
      change_state(DISPATCH);
      return;
    }
    // TODO: set a const for av off timeout
    if (ms_in_state >= kWaitForAvOffTimeoutMillis) {
      PRINTF("OFF Timeout\r\n")
      change_state(DISPATCH);
    }
    return;

    // Unexpected state.
  default:
    PRINTF("Unknown state: %d\r\n", state)
    ;
    change_state(DISPATCH);
    return;
  }
}

static void loop() {
  // Dependent module
  sense::loop();

  // Logic in this file.
  loop_led_heartbeat();
  loop_status_dump();
  loop_state();
}

int main(void) {
  setup();
  for (;;) {
    loop();
  }
}
