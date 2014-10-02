// A basic hello world example using ezlpc. Output is send to
// the USB serial port (CDC virtual com). 
//
// Using state based delay rather than a blocking delay so it can be
// extended with other functionality.

#include "ezlpc.h"

#include "usb_serial.h"
#include "system_time.h"
#include "passive_timer.h"

static PassiveTimer timer;

static void setup() {
  ezlpc::setup();
  system_time::setup();
  usb_serial::setup();

  timer.reset();
}

static void loop() {
  static int i = 0;

  if (timer.usecs() >= 1000000) {
    usb_serial::printf("Hello world: %d, %u\n", i, system_time::usecs());
    i++;
    // Reset to ideal time so we don't accumulate time error.
    timer.advance(1000000);
  }
}

int main(void) {
  setup();
  for (;;) {
    loop();
  }
}
