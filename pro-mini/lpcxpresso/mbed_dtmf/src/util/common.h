#ifndef UTIL_COMMON_H
#define UTIL_COMMON_H

#include "mbed.h"
#include "USBSerial.h"

#define ENABLE_DEBUG 0

namespace common {
extern USBSerial stdio;

inline void doNothingPrintf(const char *fmt, ...)  {
}

inline void doNothingPrint(const char *str)  {
}

}  // namepsace common

#define PRINT(str) { common::stdio.puts(str); }
#define PRINTF(...) { common::stdio.printf(__VA_ARGS__); }

#if ENABLE_DEBUG
  #define DEBUG(str) { common::stdio.puts(s); }
  #define DEBUGF(...) { common::stdio.printf(__VA_ARGS__); }
#else
  #define DEBUG(str) { common::doNothingPrint(str); }
  #define DEBUGF(...) { common::doNothingPrintf(__VA_ARGS__); }
#endif

#endif  // COMMON.H
