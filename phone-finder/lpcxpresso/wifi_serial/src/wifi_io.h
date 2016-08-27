#include "mbed.h"

#ifndef WIFI_IO_H
#define WIFI_IO_H

namespace wifi_io {

// Call once upon initialization.
extern void initialize();

// Restart the wifi module.
extern void restart();

// TX
extern bool is_putc_ready();
extern void putc(uint8_t b);

// RX
extern bool is_getc_ready();
extern uint8_t getc();

// Wifi module status.
extern bool is_wifi_irq();

}

#endif  // WIFI_IO_H
