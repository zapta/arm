// A production test program. Each output pin generates pulses
// whose period in milliseconds equals the pin number.

#include "arm_pro_mini.h"
#include "io_pins.h"
#include "system_time.h"
#include "passive_timer.h"

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
static NumberedOutputPin pin_table[] = {
  NumberedOutputPin(1, 0, 21),
  NumberedOutputPin(2, 0, 6),
  // LED on early versions boards.
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
  // LED on newer version boards.
  NumberedOutputPin(21, 0, 20),
  NumberedOutputPin(22, 0, 2),
  // This one is open drain, requires external pullup.
  NumberedOutputPin(23, 0, 4),
  NumberedOutputPin(24, 0, 10),
  NumberedOutputPin(25, 0, 3),
  // This one is open drain, requires external pullup.
  NumberedOutputPin(26, 0, 5),
};

static const int kPinCount =  sizeof(pin_table) / sizeof(pin_table[0]);

static int current_pin_index;

static PassiveTimer timer;

static void setup() {
  arm_pro_mini::setup();
  system_time::setup();
  timer.reset();
  current_pin_index = 0;
}

static void loop() {
  for (int i = 0; i < kPinCount; i++) {
    // Pins are arcive low.
    pin_table[i].set(i != current_pin_index);
  }

  if (timer.usecs() >= 750000) {
    timer.reset();
    current_pin_index = (current_pin_index + 1) % kPinCount;
  }
}

int main(void) {
  setup();
  for (;;) {
    loop();
  }
}
