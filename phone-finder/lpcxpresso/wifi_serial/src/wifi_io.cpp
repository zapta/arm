
#include "wifi_io.h"
#include "mbed.h"

// TODO: how to enable the Wifi CTS/RTS handshake?

namespace wifi_io {

// Serial data
static Serial wifi_serial(P0_19, P0_18);  // tx, rx

// Serial flow control. Signals are active low.
static DigitalOut wifi_cts_out(P0_23, 0);  // connect to wifi CTS

static DigitalIn wifi_rts_in(P0_17);  // connect to wifi RTS

// Control
static DigitalOut wifi_factory_reset_out(P0_14, 0);  // connect to wifi button1

static DigitalOut wifi_wake_out(P0_7, 0);   // connect to wifi button2

static DigitalOut wifi_reset_out(P0_11, 0);  // connect to wifi N_RESET

// Status
static DigitalIn wifi_irq_in(P0_2);  // connect to wifi gpio 16 (configurable)

void initialize() {
  wifi_serial.baud(115200);
  restart();
}

void restart() {
  wifi_reset_out = false;  // inactive
  wifi_factory_reset_out = false;  // inactive
  wifi_wake_out = false;  // inactive

  while (wifi_serial.readable()) {
    wifi_serial.getc();
    wait_ms(1);  // wait in case another char is on the way.
  }

  // TODO: try shorter delay (e.g. 5ms)
  wait_ms(100);
  wifi_reset_out = true;

}

bool is_putc_ready() {
  return wifi_serial.writeable() && !wifi_rts_in;
}

void putc(uint8_t b) {
  wifi_serial.putc(b);
}

bool is_getc_ready() {
  return wifi_serial.readable();
}
uint8_t getc() {
  return wifi_serial.getc();
}

bool is_wifi_irq() {
  return wifi_irq_in.read();
}

}  // namespace wifi_io
