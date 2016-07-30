#ifndef DTMF_H
#define DTMF_H

#include "mbed.h"

namespace dtmf {

const int kMaxDialingSequence = 20;

// Call once upon initialization.
extern void initialize();

// Call from main loop.
extern void loop();

// Start dialing given sequence of dtmf codes (see dtmf_io.cpp for code table).
// dtmf_codes is copied to an internal dialing buffer and not being accessed
// after return. If is_idle() is false or if the sequence is longer than
// kMaxDialingSequence this call is ignored silently.
extern void start_dialing(const char* dtmf_codes);

extern bool is_idle();

}  // dtmf

#endif  // DTMF_H
