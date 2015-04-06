// A think wraper around system time to measure elapsed time in microseconds.

#ifndef PASSIVE_TIMER_H
#define PASSIVE_TIMER_H

#include "system_time.h"

class PassiveTimer {
public:
  PassiveTimer() {
    reset();
  }

  // Start as of now.
  inline void reset() {
    _start_time_usecs = system_time::usecs64();
  }

  // Return time in microseconds since last start.
  // Wraps around after ~70 minutes after reset.
  // More efficient than millis() and secs() so prefer when possible.
  inline uint32_t usecs() const {
    return static_cast<uint32_t>(system_time::usecs64() - _start_time_usecs);
  }

  // Return time in milliseconds since last start.
  // Wraps around after ~50 days after reset.
  // Less efficient than usecs().
  inline uint32_t millis() const {
    return static_cast<uint32_t>(
        (system_time::usecs64() - _start_time_usecs) / 1000);
  }

  // Return time in seconds since last start.
  // Wraps around after ~135 years after reset.
  // Less efficient than usecs().
  inline uint32_t secs() const {
    return static_cast<uint32_t>(
        (system_time::usecs64() - _start_time_usecs) / 1000000);
  }

private:
  uint64_t _start_time_usecs;
};

#endif
