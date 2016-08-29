// DTMF dialer.

#ifndef DIALER_DTMF_H
#define DIALER_DTMF_H

#include "mbed.h"

namespace dtmf {

//const int kMaxDialingSequence = 20;

// Call once upon initialization.
extern void initialize();

// Call from main loop.
extern void loop();

// Start dialing given sequence of dtmf codes (see dtmf_io.cpp for code table).
// dtmf_code buffer should stay stable until dialing is_idle() is true.
extern void start_dialing(const char* dtmf_codes);

extern bool is_dialing_in_progress();

// While is_dialing_in_progress() is true LED should be controlled
// by this signal.
extern bool led_control();

// Force continuous dtmf code. If any dialing is in progress abort it.
// Useful for testing.
extern void force_continuous_code(char dtmf_code);

}  // dtmf

#endif  // DIALER_DTMF_H
