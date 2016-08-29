#include <phone/dialer.h>
#include <phone/dtmf.h>
#include <phone/hook.h>
#include "util/common.h"


namespace dialer {

// NOTE: my cell phone stop ringing after ~25 secs.
const int kCallHoldTimeMillis   = 25000;

enum State {
  //
  IDLE,
  //
  ACTIVATE,
  //
  DIALING,
  //
  CALL_DELAY,
  //
  DEACTIVATE
};

static State state = DEACTIVATE;

// For debugging.
static const char* state_name(State state) {
  switch (state) {
    case IDLE: return "IDLE";
    case ACTIVATE: return "ACTIVATE";
    case DIALING: return "DIALING";
    case CALL_DELAY: return "CALL_DELAY";
    case DEACTIVATE: return "DEACTIVATE";
    default: return "???";
  }
}

// Ignored in IDLE state.
static const char* number_to_dial = "";

// Time in current state. Invalid in IDLE state due to possible
// timer overflow.
static Timer time_in_state;

// For periodic beeps during call delay.
static Timer beep_timer;

// See dtmf.h
void initialize() {
  time_in_state.start();
  beep_timer.start();

  dtmf::initialize();
  hook::initialize();
}

static void change_state(State new_state) {
  PRINTF("%s -> %s\r\n", state_name(state), state_name(new_state));
  time_in_state.reset();
  // We always reset the beep counter even though we need it only in
  // CALL_DELAY state.
  beep_timer.reset();
  state = new_state;
}

// See dtmf.h
void loop() {
  dtmf::loop();
  hook::loop();

  switch (state) {
    case IDLE:
      hook::set_hook(false);
      return;

    case ACTIVATE:
      hook::set_hook(true);
      if (hook::is_hook_stable_on()) {
        change_state(DIALING);
        dtmf::start_dialing(number_to_dial);
      }
      return;

    case DIALING:
      if (!dtmf::is_dialing_in_progress()) {
        change_state(CALL_DELAY);
      }
      return;

    case CALL_DELAY:
      // Test if time to end the call.
      if (time_in_state.read_ms() > kCallHoldTimeMillis) {
        change_state(DEACTIVATE);
        return;
      }
      // Periodic beeps during the call, to indicate the nature of the call.
      if (beep_timer.read_ms() > 3000) {
        beep_timer.reset();
        PRINTF("** BEEP **\r\n");
        // NOTE: The phone/cell company seems to filter out valid DTMF tones.
        // For this reason, we use for the beep a single tone that doesn't
        // colides with DTMF tones.
        dtmf::start_dialing("BBB");
      }
      return;

    case DEACTIVATE:
      hook::set_hook(false);
      if (hook::is_hook_stable_off()) {
        change_state(IDLE);
      }
      return;

    default:
      PRINTF("Unknown dialer state: %d\r\n", state);
      change_state(IDLE);
  }
}

// See dialer.h.
void call(const char* number) {
  if (state != IDLE) {
    PRINTF("A call already in progress\r\n");
    return;
  }
  number_to_dial = number;
  change_state(ACTIVATE);
}

// See dialer.h.
bool is_call_in_progress() {
  return state != IDLE;
}

bool led_control() {
  switch (state) {
    case IDLE:
      return false;

    case ACTIVATE:
      return true;

    case DIALING:
      return dtmf::led_control();

    case CALL_DELAY:
      // Fast 3Hz blink
      return (time_in_state.read_ms() % 300) < 100;

    case DEACTIVATE:
      return false;

    default:
      return false;
  }
}

}  // dialer

