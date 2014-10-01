

#ifndef USB_SERIAL_H
#define USB_SERIAL_H

namespace usb_serial {

  // Call from main setup() and loop() respectivly.
  extern void setup();
//  extern void loop();

  // Momentary size of free space in the output buffer. Sending at most this number
  // of characters will not loose any byte.
//  extern uint8 capacity();
//
//  extern void printchar(uint8 b);
//  extern void print(const __FlashStringHelper *str);
//  extern void println(const __FlashStringHelper *str);
//  extern void print(const char* str);
//  extern void println(const char* str);
//  extern void println();
  extern void printf(const char *format, ...);
//  extern void printhex2(uint8 b);

  // Wait in a busy loop until all bytes were flushed to the UART.
  // Avoid using this when possible. Useful when needing to print
  // during setup() more than the output buffer can contain.
  //void waitUntilFlushed();
}  // namespace us_serial

#endif
