#ifndef RGB_IO_H
#define RGB_IO_H

#include "mbed.h"

namespace rgb_io {

extern void initialize();

extern void set(uint8_t r, uint8_t g, uint8_t b);

}  // rgb_io

#endif  // RGB_IO_H
