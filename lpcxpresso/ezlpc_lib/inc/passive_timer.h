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
    start_time_usecs_ = system_time::usecs();
  }

  // Start at last start time + given increment.
  inline void advance(uint32 delta_time_usecs) {
    start_time_usecs_ += delta_time_usecs;
  }

  // Copy from another instance.
  void copy(const PassiveTimer &other) {
    start_time_usecs_ = other.start_time_usecs_;
  }

  // Return time in usec since last start.
  inline uint32 usecs() const {
    return system_time::usecs() - start_time_usecs_;
  }

private:
  uint32 start_time_usecs_;
};

#endif
