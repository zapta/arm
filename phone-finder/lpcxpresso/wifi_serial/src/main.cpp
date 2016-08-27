// A simple mbed program for Arm Pro Mini + Ack.Me Wifi module.
// The program proxies the serial data between the USB serial (host computer)
// and the Wifi module.
//
// Tested on LPCXpresso 8.0.0 on Dec 2015.

#include "mbed.h"
#include "USBSerial.h"
#include "rgb_io.h"
#include "wifi_io.h"

// ARM PRO MINI red LED is at GPIO0_20.
static DigitalOut led(P0_20, 0);

static Timer led_timer;

USBSerial usb_serial(0x1f00, 0x2012, 0x0001, false);

static Timer tx_timer;
static Timer rx_timer;

static void setup() {
  led_timer.start();
  tx_timer.start();
  rx_timer.start();

  rgb_io::initialize();
  wifi_io::initialize();
}

static bool last_tx_status = false;;
static bool last_rx_status = false;;

static void loop() {
  // Blinking red led.
  const int led_timer_ms = led_timer.read_ms();
  led = (led_timer_ms <= 50);
  if (led_timer_ms >= 1000) {
    led_timer.reset();
  }

  // USB to Wifi
  while (usb_serial.readable() && wifi_io::is_putc_ready() ) {
    const int c = usb_serial.getc();
    wifi_io::putc(c);
    tx_timer.reset();
  }

  // Wifi to USB
  while (wifi_io::is_getc_ready() && usb_serial.writeable()) {
     const int c = wifi_io::getc();
     usb_serial.putc(c);
     rx_timer.reset();
  }

  // Update RGB if needed
  const bool tx_status = tx_timer.read_ms() <= 100;
  const bool rx_status = rx_timer.read_ms() <= 100;

  led = tx_status || rx_status;

  if ((tx_status != last_tx_status) || (rx_status != last_rx_status)) {
    last_tx_status = tx_status;
    last_rx_status = rx_status;
    rgb_io::set(
        tx_status ? 0x10 : 0x00,  // tx -> red
        rx_status ? 0x10 : 0x00,  // rx -> green
        0x00);                    // blue off
  }
}

int main(void) {
  setup();
  for (;;) {
    loop();
  }
}
