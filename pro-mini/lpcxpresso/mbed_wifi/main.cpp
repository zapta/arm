#include "mbed.h"
#include "USBSerial.h"
 
DigitalOut led(P0_7);  // LED pin on my board.
 
// Virtual serial port over USB.
// Using default USBSerial device parameters except for 'flase' for
// non blocking if USB is not connected (e.g. when running on external
// power supply).
USBSerial usb_serial(0x1f00, 0x2012, 0x0001, false);

// Hardware serial to wifi device.
Serial wifi_serial(P0_19, P0_18);  // tx, rx

enum state {
  INITIAL_DELAY = 1,
  OPERATING = 2,
};

static int state;

Timer timer;

void setup() {
  timer.start();
  //wifi_serial.baud(115200);
  wifi_serial.baud(9600);
  wifi_serial.format(8, SerialBase::None, 1);
  state = INITIAL_DELAY;
}

void wifi_serial_loop() {
  while(wifi_serial.readable()) {
    const char c = wifi_serial.getc();
    //usb_serial.putc('[');
    usb_serial.putc(c);
    //usb_serial.putc(']');
  }
  //usb_serial.printf("[%c]\r\n", c);
}


void loop() {
  const int time_in_cycle_ms = timer.read_ms();

  // Handle INITIAL_DELAY state.
  if (state == INITIAL_DELAY) {
    if (time_in_cycle_ms < 2000) {
      return;
    }
    state = OPERATING;
    timer.reset();
    return;
  }

  // Handle OPERATING STATE.
  wifi_serial_loop();

  led = (time_in_cycle_ms < 20);

  if (time_in_cycle_ms < 1000) {
      return;
  }
  timer.reset();
  usb_serial.printf("----\n");
  wifi_serial.printf("AT\r\n");
}

int main(void) {
  setup();
  for(;;) {
    loop();
  }
}
