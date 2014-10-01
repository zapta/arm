// A basic hello world example using ezlpc. Output is send to
// the USB serial port (CDC virtual com).

#include "ezlpc.h"

#include "usb_serial.h"
#include "system_time.h"

int main(void) {
  // Initialization
  ezlpc::setup();
  usb_serial::setup();

  // Actual program
  for (int i = 0; ; i++) {
    usb_serial::printf("Hello world: %d, %04x, %d\n", i, i, 10000 / i);
    system_time::delay_ms(1000);
  }
}
