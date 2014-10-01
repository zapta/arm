
#include "system_time.h"

namespace system_time {

// TODO(zapta): have a more accurate time/delay.
void delay_ms(uint16 ms) {
  uint16 delay;
  volatile uint32 i;
  for (delay = ms; delay > 0; delay--) {
    // ~1ms loop with -Os optimisatio
    for (i = 3500; i > 0; i--) {
    }
  }
}

}  // namespace system_time
