// Driver for the AckMe Wifi module. This is the only public
// API provided by files in this directory.

#ifndef WIFI_WIFI_H
#define WIFI_WIFI_H

#include "mbed.h"

namespace wifi {

// Call once to initialize.
extern void initialize();

// Call from the main polling loop.
extern void polling();

// Dump internal information for debugging. Prints to the debug output.
extern void dumpInternalState();

// Call this on any unrecoverable connection error. This will trigger a cycle
// of reconnecting.
extern void protocolPanic(const char* short_message);

// Return a bit set of currently pressed buttons. First button is
// LSB. Corresponds to button list.
extern int getPressedButtonSet();

}  // namespace wifi

#endif  // WIFI_WIFI_H
