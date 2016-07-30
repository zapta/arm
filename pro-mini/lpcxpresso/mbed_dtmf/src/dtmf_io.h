#ifndef DTMF_IO_H
#define DTMF_IO_H

#include "mbed.h"

namespace dtmf_io {

// Upon return tone is off.
extern void initialize();

// 0-9 : decimal digits
// 10  : '#'
// 11  : '*'
extern void set_tone(int tone_index);

// Turn off tone.
extern void tone_off();

}  // dtmf_io

#endif  // DTMF_IO_H
