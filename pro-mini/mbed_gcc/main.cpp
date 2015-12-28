#include "mbed.h"
#include "USBSerial.h"
 
DigitalOut led(P0_20, 1);  // LED pin
 
//Virtual serial port over USB
USBSerial usb_serial;

// Hardware serial to wifi device.
Serial wifi_serial(P0_19, P0_18);  // tx, rx

Timer timer;

void setup() {
  timer.start();
  wifi_serial.baud(19200);
}

void loop() {
  if (timer.read_ms() < 100) {
      return;
  }
led = !led;    // Invert LED state
  timer.reset();
  usb_serial.printf("usb\r\n");
  wifi_serial.printf("wifi\r\n");
}

int main(void) {
  setup();
  for(;;) {
    loop();
  }
}
