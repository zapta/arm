#include "dialer/dtmf.h"

#include "util/common.h"
#include "util/status_led.h"
#include "dialer/dtmf_io.h"

//#include "USBSerial.h"

//extern USBSerial usb_serial;

//extern DigitalOut led1;

namespace dtmf {

const int kSpaceTimeMillis =  40;
const int kMarkTimeMillis  = 40;

enum State {
  // Not dialing
  IDLE,
  // First step of dialing a code (tones off)
  SPACE,
  // Second step of dialing a code (tones on)
  MARK,
};

static State state = IDLE;

// Used to time marks and spaces.
static Timer timer;

// When dialing, contains pointer to the dtmf codes to dial.
// Ignored when state is IDLE.
static const char* dialing_buffer = "";

// Dialing codes buffer. Null terminated.
//static char dialing_buffer[kMaxDialingSequence+1] = "";

// Index of current code in dialing_buffer. Valid in SPACE and MARK
// states. Ignored in IDLE state.
static int current_code_index = 0;

// See dtmf.h
void initialize() {
  timer.start();
  dtmf_io::initialize();
}

// See dtmf.h
void loop() {
  switch (state) {
    case IDLE:
      return;
      break;

    case SPACE:
      if (timer.read_ms() < kSpaceTimeMillis) {
        return;
      }
      PRINTF("S->M %d\r\n", current_code_index);
      state = MARK;
      timer.reset();
      dtmf_io::set_dtmf_code(dialing_buffer[current_code_index]);
      break;

    case MARK:
      status_led::led_pin = 1;
      if (timer.read_ms() < kMarkTimeMillis) {
        return;
      }
      dtmf_io::set_dtmf_code(' ');
      current_code_index++;
      if (!dialing_buffer[current_code_index]) {
        state = IDLE;
        PRINTF("M->I %d\r\n", current_code_index);
        return;
      }
      state = SPACE;
      PRINTF("M->S %d\r\n", current_code_index);
      timer.reset();
      break;

    default:
      dtmf_io::set_dtmf_code(' ');
      state = IDLE;
  }
}

// See dtmf.h
void start_dialing(const char* dtmf_codes) {
  PRINTF("***DIAL: [%s]\r\n", dtmf_codes);

  // If not idle then ignore.
  if (state != IDLE) {
    return;
  }

  // If sequence is empty or too long then ignore
  const int n = strlen(dtmf_codes);
  if (n < 1 || n >= 30) {
    PRINTF("Ignoring: len=%d\r\n", n);
    return;
  }

  dialing_buffer = dtmf_codes;

  // Copy sequence to buffer.
  //memcpy(dialing_buffer, dtmf_codes, n+1);

  // Start. For first code we skip the SPACE and go directly to the
  // MARK.
  state = MARK;
  current_code_index = 0;
  PRINTF("I->M %d\r\n", current_code_index);
  timer.reset();
  dtmf_io::set_dtmf_code(dialing_buffer[current_code_index]);
}

// See dtmf.h
bool is_dialing_in_progress() {
  return (state != IDLE);
}

// See dtmf.h
void force_continuous_code(char dtmf_code) {
  state = IDLE;
  dtmf_io::set_dtmf_code(dtmf_code);
}

}  // dtmf

