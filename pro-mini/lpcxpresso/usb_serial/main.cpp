#include "mbed.h"
#include "USBSerial.h"

static Serial serial(P0_19, P0_18);  // tx, rx

static USBSerial usb(0x1f00, 0x2012, 0x0001, false);

static DigitalOut led(P0_20, 0);
static Timer timer;

static void setup() {
  serial.baud(9600);
  serial.format(8, SerialBase::None, 1);
  led_timer.start();
}

static void loop() {
  led.write(timer.read_ms() < 50);

  while (serial.readable()) {
    usb.putc(serial.getc());
    led_timer.reset();
  }

  while (usb.readable()) {
    serial.putc(usb.getc());
    timer.reset();
  }
}

// Arduino like main
int main(void) {
  setup();
  for (;;) {
    loop();
  }
}
