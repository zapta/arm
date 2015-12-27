#include "system_time.h"

#include "chip/clock_11xx.h"

// Using CT32B1 timer, similar to mbed.
#define SYS_CLOCK_TIMER32 (LPC_TIMER32_1)

namespace system_time {

void setup() {
  const uint32 system_clock_hz = Chip_Clock_GetSystemClockRate();
  const uint32 prescale = system_clock_hz / 1000000;

  Chip_TIMER_Init(SYS_CLOCK_TIMER32);

    // Set prescaler for 1 usec/count.
  Chip_TIMER_PrescaleSet(SYS_CLOCK_TIMER32, prescale - 1);
  // Reset count
  Chip_TIMER_Reset(SYS_CLOCK_TIMER32);
  // Start counting.
  Chip_TIMER_Enable(SYS_CLOCK_TIMER32);
}

uint32 usecs() {
  return Chip_TIMER_ReadCount(SYS_CLOCK_TIMER32);
}


// TODO: compensate for the function call/return time?
void delay_usec(uint32 delay_time_usec) {
  const uint32 start_time = usecs();
  while (usecs() - start_time < delay_time_usec) {
    // Do nothing.
  }
}

}  // namespace system_time
