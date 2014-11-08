// A basic hello world example using the ezlpc library. Text output is sent to
// the USB serial port (CDC virtual com). 

#include "arm_pro_mini.h"

// Provides abstraction to digital I/o pins.
#include "io_pins.h"

// Provide system time using timer xxx.
#include "system_time.h"

// Provides interrupt free elapsed time measurement using system time.
#include "passive_timer.h"

// Provides serial I/O over USB/CDC.
#include "usb_serial.h"

// Timer for generating the delay bettween printed messages.
static PassiveTimer timer;

// Red LED is at GPIO0_7.
static io_pins::OutputPin led(0, 7);

static void setup() {
  arm_pro_mini::setup();
  // Uses timer32_0 for generating a 1 usec 32 bit clock (does not use interrupts).
  system_time::setup();
  // Initialize the USB serial connection. This will allow us to print messages.
  usb_serial::setup();
  // Reset the timer to the time now. This starts the first cycle.
  timer.reset();
}

static void loop() {
  static int message_count = 0;

  const uint32 time_now_usecs = timer.usecs();

  const uint32 cycle_time_usecs = 200000;

  // Generates a short blink at the beginning of each cycle.
  led.set(time_now_usecs <= cycle_time_usecs / 3);
  if (time_now_usecs >= cycle_time_usecs) {
    usb_serial::printf("Hello world: %d, %u\n", message_count++,
        system_time::usecs());
    // Advance cycle start time rather than reseting to time now. This
    // way we don't accumulate time errors.
    timer.advance_start_time_usecs(cycle_time_usecs);
  }
}

int main(void) {
  setup();
  for (;;) {
    loop();
  }
}
