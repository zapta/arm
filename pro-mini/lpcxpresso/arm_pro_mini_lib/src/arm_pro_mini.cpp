
#include "arm_pro_mini.h"

#include "chip.h"

/* System oscillator rate and clock rate on the CLKIN pin */
const uint32_t OscRateIn = 12000000;
const uint32_t ExtRateIn = 0;

namespace arm_pro_mini {

  void setup() {
    SystemCoreClockUpdate();
    Chip_GPIO_Init(LPC_GPIO);
  }
} // namespace arm_pro_mini
