#ifndef DTMF_IO_H
#define DTMF_IO_H

#include "mbed.h"

namespace dtmf_io {

// Upon return tone is off.
extern void initialize();

// See dtmf_id for the dtmf_ascii_code table.
// Use code ' ' to disable.
extern void set_dtmf_code(char dtmf_ascii_code);

}  // dtmf_io

#endif  // DTMF_IO_H
