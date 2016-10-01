#include "sense.h"
#include "common.h"

#include "mbed.h"

namespace sense {

static int kDebouncingTimeMillis = 100;

// Input pins, active low.
static DigitalIn av_in_pin(P0_10, PullUp);
static DigitalIn tv_in_pin(P0_11, PullUp);

// Timers for debouncing.
static Timer av_timer;
static Timer tv_timer;

// Flags, post debouncing.
static bool av_on = false;
static bool tv_on = false;

void setup() {
  av_timer.start();
  tv_timer.start();
}

void loop() {
  // Update av sensing.
  {
    const bool signal = !av_in_pin;  // active low
    if (signal != av_on) {
      if (av_timer.read_ms() >= kDebouncingTimeMillis) {
        av_on = signal;
        av_timer.reset();
        DEBUGF("AV -> %s\r\n", av_on ? "ON" : "OFF");
      }
    } else {
      av_timer.reset();
    }
  }

  // Update tv sensing.
  {
    const bool signal = !tv_in_pin;  // active low
    if (signal != tv_on) {
      if (tv_timer.read_ms() >= kDebouncingTimeMillis) {
        tv_on = signal;
        tv_timer.reset();
        DEBUGF("TV -> %s\r\n", tv_on ? "ON" : "OFF");
      }
    } else {
      tv_timer.reset();
    }
  }
}

bool is_av_on() {
  return av_on;
}

bool is_tv_on() {
  return tv_on;
}

}  // namespace sense
