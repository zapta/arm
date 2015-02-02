// A production test program. Each output pin generates pulses
// whose period in milliseconds equals the pin number.

// The base arm_pro_mini_lib include.
#include "arm_pro_mini.h"

// Provides abstraction to digital I/o pins.
#include "io_pins.h"

// Provide system time using TIMER32 0.
#include "system_time.h"

// Provides interrupt free elapsed time measurement using system time.
#include "passive_timer.h"

// Allows to jump to ISP mode when ISP button is pressed.
#include "isp_button_monitor.h"

// Timer for timing the cycles.
static PassiveTimer timer;

// Extends an output pin with a physical pin number.
class NumberedOutputPin : public io_pins::OutputPin {
public:
  NumberedOutputPin(uint8 pin_number, uint8 port_index, uint8 bit_index)
    : io_pins::OutputPin(port_index, bit_index),
      pin_number(pin_number) {}
  const uint8 pin_number;
};

// Output pins to test. Each pins generates a pulse whose width
// in milliseconds is the physical pin number.
static NumberedOutputPin pinTable[] = {
  NumberedOutputPin(1, 0, 21),
  NumberedOutputPin(2, 0, 6),
  NumberedOutputPin(3, 0, 7),
  NumberedOutputPin(4, 0, 8),
  NumberedOutputPin(5, 0, 9),
  NumberedOutputPin(6, 0, 22),
  NumberedOutputPin(7, 0, 11),
  NumberedOutputPin(8, 0, 12),
  NumberedOutputPin(9, 0, 13),
  NumberedOutputPin(10, 0, 14),
  NumberedOutputPin(11, 0, 15),
  NumberedOutputPin(12, 0, 16),
  NumberedOutputPin(13, 0, 23),
  NumberedOutputPin(14, 1, 15),
  NumberedOutputPin(15, 0, 17),
  NumberedOutputPin(16, 0, 18),
  // ISP button, avoid short to ground.
  // NumberedOutputPin(17, 0, 1)
  NumberedOutputPin(18, 0, 19),
  NumberedOutputPin(19, 1, 19),
  // RST button, avoid short to ground.
  // NumberedOutputPin(20, 0, 0),
  NumberedOutputPin(21, 0, 20),
  NumberedOutputPin(22, 0, 2),
  // This one is open drain, requires external pullup.
  NumberedOutputPin(23, 0, 4),
  NumberedOutputPin(24, 0, 10),
  NumberedOutputPin(25, 0, 3),
  // This one is open drain, requires external pullup.
  NumberedOutputPin(26, 0, 5),
};

static const int kPinCount =  sizeof(pinTable) / sizeof(pinTable[0]);

static void setup() {
  arm_pro_mini::setup();
  // Uses timer32_0 for generating a 1 usec 32 bit clock (does not use interrupts).
  system_time::setup();
  // Get ready to monitor the ISP button
  isp_button_monitor::setup();
  // Reset the timer to the time now. This starts the first cycle.
  timer.reset();
}

static void loop() {
  // If the ISP button is pressed, this will jump to the USB/ISP
  // mode, allowing to upgrade the firmware via drag and drop.
  // Otherwise just reset the board while the ISP button is pressed.
  isp_button_monitor::loop();

  const uint32 time_now_in_cycle_usecs = timer.usecs();
  const uint32 time_now_in_cycle_msecs = time_now_in_cycle_usecs / 1000;

  // Update all pins based on their pin number and the time in current
  // cycle.
  for (int i = 0; i < kPinCount; i++) {
    NumberedOutputPin& pin = pinTable[i];
    pin.set(pin.pin_number > time_now_in_cycle_msecs);
  }

  // Check if current cycle done.
  if (time_now_in_cycle_msecs >= 50) {
    timer.reset();
  }
}

int main(void) {
  setup();
  for (;;) {
    loop();
  }
}
