#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H

#include "mbed.h"

// Provide 64bit usec time since reboot.
namespace system_time {

// Initializes the the 32 bit timer.
extern void initialize();

// Update the internal time. Should be called periodically
// so we can handle property the overflow of the underlying
// uint32 usec hardware clock.
extern void polling();

// Return time in usecs at the last call to polling().
extern uint64_t usecs64();

}  // namespace system_time

#endif
