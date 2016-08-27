#include "util/system_time.h"

#include "util/common.h"
#include "us_ticker_api.h"

namespace system_time {

// Usecs time at the last call to polling() (or initialize() if polling was not called)
static uint64_t time_usecs;

// Last read from the underlying uint32 usecs clock.
static uint32_t last_usec_clock_time_usecs;

void initialize() {
  last_usec_clock_time_usecs = us_ticker_read();
  time_usecs = last_usec_clock_time_usecs;
}

void polling() {
  const uint32_t prev_usec_clock_time_usecs = last_usec_clock_time_usecs;
  last_usec_clock_time_usecs = us_ticker_read();
  // NOTE: this should give the correct delta also after a uint32 wrap around.
  const uint32_t delta_usecs = (last_usec_clock_time_usecs - prev_usec_clock_time_usecs);
  time_usecs += delta_usecs;
}

uint64_t usecs64() {
  return time_usecs;
}

}  // namespace system_time
