#include <src/dtmf.h>
#include <src/dtmf_io.h>
//#include "cmsis.h"
//#include "pinmap.h"
#include "USBSerial.h"


extern USBSerial usb_serial;

namespace dtmf {

// TODO: shorten periods to the standard.
const int kSpaceTimeMillis = 100;
const int kMarkTimeMillis = 200;

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

// Dialing codes buffer. Null terminated.
static char dialing_buffer[kMaxDialingSequence+1] = "";

// Index of current code in dialing_buffer. Valid in SPACE and MARK
// states.
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
      usb_serial.printf("S-M %d\r\n", current_code_index);
      state = MARK;
      timer.reset();
      dtmf_io::set_dtmf_code(dialing_buffer[current_code_index]);
      break;

    case MARK:
      if (timer.read_ms() < kMarkTimeMillis) {
        return;
      }
      dtmf_io::set_dtmf_code(' ');
      current_code_index++;
      if (!dialing_buffer[current_code_index]) {
        state = IDLE;
        return;
      }
      state = SPACE;
      timer.reset();
      break;

    default:
      dtmf_io::set_dtmf_code(' ');
      state = IDLE;
  }
}

// See dtmf.h
void start_dialing(const char* dtmf_codes) {
  usb_serial.printf("***DIAL: [%s]\r\n", dtmf_codes);

  // If not idle then ignore.
  if (state != IDLE) {
    return;
  }

  // If sequence is empty too long then ignore
  const int n = strlen(dtmf_codes);
  if (n < 1 || n >= kMaxDialingSequence) {
    return;
  }

  // Copy sequence to buffer.
  memcpy(dialing_buffer, dtmf_codes, n+1);

  // Start. For first code we skip the SPACE and go directly to the
  // MARK.
  state = MARK;
  current_code_index = 0;
  timer.reset();
  dtmf_io::set_dtmf_code(dialing_buffer[current_code_index]);
}

// See dtmf.h
bool is_idle() {
  return (state == IDLE);
}

}  // dtmf

