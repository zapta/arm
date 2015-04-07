// Driver for the ESP8266 with the Lua persistent connection app.

#ifndef ESP8266_H
#define ESP8266_H

#include "byte_fifo.h"
#include "byte_fifo_uart_rx.h"

namespace esp8266 {

// Call first to initialize.
extern void initialize();

// Call from the main loop()
extern void polling();

// TODO: hide the fifos and provide basic access functions.

// Read downstream bytes from here.
extern ByteFifo rx_fifo;

// Push upstream bytes to here.
extern ByteFifo tx_fifo;

// Return the current connection id (non zero) or zero if not connected.
extern uint32_t connectionId();

// If connected then close the current connectiona and reconnect.
// Otherwise do nothing. Usefull for situation when the network protocol
// encoutnered an unrecoverable error.
extern void abortCurrentConnection();

// Dump internal information for debugging.
extern void dumpInternalState();

// TODO: remove, for initial debuggin only.
extern bool had_bad_line;

}  // namespace esp8266

#endif  // ESP8266_H
