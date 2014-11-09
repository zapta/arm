#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H

#include "arm_pro_mini.h"

namespace system_time {

// Initializes the the 32 bit timer.
extern void setup();

// Return the current time in usec modulo 32 bit (20+ Days cycle).
// TODO: consider to inline.
extern uint32 usecs();

// Delay for given time in usec.
extern void delay_usec(uint32 time_usec);

}  // namespace system_time

#endif
