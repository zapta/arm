// A basic hello world example using the arm_pro_mini library. It blinks the LED
// and prints to the USB/CDC serial port.

#include "arm_pro_mini.h"

// Provides abstraction to digital I/o pins.
#include "io_pins.h"

// Provide system time using TIMER32 0.
#include "system_time.h"

// Provides interrupt free elapsed time measurement using system time.
#include "passive_timer.h"

// Provides serial I/O over USB/CDC.
#include "usb_serial.h"

// Allows to jump to ISP mode when ISP button is pressed.
#include "isp_button_monitor.h"

// LED blink cycle. We provide two prebuilt binaries with
// fast and slow blink respectively to be used in the
// Getting Started procedure.
//
static const uint32 kCycleTimeUsecs = 300 * 1000;  // fast
// static const uint32 kCycleTimeUsecs = 2000 * 1000;  // slow

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
  // Get ready to monitor the ISP button
  isp_button_monitor::setup();
  // Reset the timer to the time now. This starts the first cycle.
  timer.reset();
}

static void loop() {
  // If the ISP button is pressed, this will jump to the USB/ISP
  // mode, allowing to upgrade the firmware via drag and drop.
  isp_button_monitor::loop();

  static int message_count = 0;
  const uint32 time_now_in_cycle_usecs = timer.usecs();

  // Generates a blink at the beginning of each cycle.
  led.set(time_now_in_cycle_usecs <= kCycleTimeUsecs / 3);
  if (time_now_in_cycle_usecs >= kCycleTimeUsecs) {
    usb_serial::printf("Hello world: %d, %u\n", message_count,
        system_time::usecs());
    message_count++;
    // Advance cycle start time rather than reseting to time now. This
    // way we don't accumulate time errors.
    timer.advance_start_time_usecs(kCycleTimeUsecs);
  }
}

int main(void) {
  setup();
  for (;;) {
    loop();
  }
}
