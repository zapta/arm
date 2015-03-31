
#include "debug.h"

// Non blocking. Will complete the initialization even if not connected
// to a host.
USBSerial debug(0x1f00, 0x2012, 0x0001, false);
