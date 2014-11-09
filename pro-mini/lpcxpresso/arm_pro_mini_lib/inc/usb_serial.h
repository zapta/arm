// Serial over USB (CDC) api.

#ifndef USB_SERIAL_H
#define USB_SERIAL_H

namespace usb_serial {
  // Call from main setup() and loop() respectivly.
  extern void setup();

  // Output text.
  extern void printf(const char *format, ...);

  // TODO: add input function.

}  // namespace usb_serial

#endif  // USB_SERIAL_H
