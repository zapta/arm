// Phone dialer.

#ifndef DIALER_H
#define DIALER_H

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

}  // dialer

#endif  // DIALER_H
