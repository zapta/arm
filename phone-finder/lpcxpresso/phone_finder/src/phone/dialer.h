// Phone dialer.

#ifndef DIALER_DIALER_H
#define DIALER_DIALER_H

#include "mbed.h"

namespace dialer {

// Call once upon initialization.
extern void initialize();

// Call from main loop.
extern void loop();

// Call the given number, stay connected for given number of seconds after
// completing the dialing. number should stay stable until is_calling() is
// false;
extern void call(const char* number);

// Return true if a call is in progress.
extern bool is_call_in_progress();

// While is_call_in_progress() is true LED should be controlled
// by this signal.
extern bool led_control();

}  // dialer

#endif  // DIALER_DIALER_H
