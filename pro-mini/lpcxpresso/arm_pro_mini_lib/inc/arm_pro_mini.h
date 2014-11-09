// Main include file of the ARM PRO MINI library.

#ifndef __ARM_PRO_MINI_H_
#define __ARM_PRO_MINI_H_

#include "chip/lpc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Define friendlier basic  type names.
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

namespace arm_pro_mini {
  // Call this once to intialize the board.
  extern void setup();

  // Jump to ISP mode. Never returns.
  void ReinvokeISP();
}

#ifdef __cplusplus
}
#endif

#endif /* __ARM_PRO_MINI_H_ */
