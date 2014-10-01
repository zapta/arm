// A basic hello world example using ezlpc. Output is send to
// the USB serial port (CDC virtual com).

#include "ezlpc.h"

#include "usb_serial.h"

// TODO(zapta): have a more accurate time/delay.
void _delay_ms(uint16 ms) {
  uint16 delay;
  volatile uint32 i;
  for (delay = ms; delay > 0; delay--) {
    // ~1ms loop with -Os optimisatio
    for (i = 3500; i > 0; i--) {
    }
  }
}

int main(void) {
  // Initialization
  ezlpc::setup();
  usb_serial::setup();

  // Actual program
  int i = 0;
  for (;;) {
    i++;
    usb_serial::printf("Hello world: %d, %04x, %d\n", i, i, 10000 / i);
    _delay_ms(1000);
  }
}
