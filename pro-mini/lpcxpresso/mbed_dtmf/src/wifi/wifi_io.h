// I/O to/from the AckMe WIFI module.
//
// Connectivity
// ------------
// RESET
// WAKE
// FACTORY_RESET
// SERIAL TX
// SERIAL RX
// SERIAL RTS
// SERIAL CTS
// STREAM_DATA_IRQ

#ifndef WIFI_WIFI_IO_H
#define WIFI_WIFI_IO_H

#include "util/common.h"

namespace wifi_io {

// ----- Housekeeping

// Call once upon initialization.
extern void initialize();

// Call from the main polling loop.
extern void polling();

// Prints a line with summary of the internal state.
extern void dumpInternalState();


// ----- RX (from the WIFI module)

// Remove any pending bytes from the rx buffer and clear the rx_overflow flag if set.
extern void rx_reset();

// Returns true if the rx buffer was overflowed since the last reset.
extern bool is_rx_overflow();

// Try to read a single byte from the rx buffer. If returns true, *byte is set
// to the new byte.
extern bool rx_read_byte(uint8_t* byte);


// ----- TX (to the WIFI module)

// Returns the number of free bytes in the tx buffer. If the returned
// value is non zero, the tx buffer is guaranteed to accept at least
// this number of bytes.
extern int tx_free_bytes();

// Test if the tx buffer is empty. When this is true, the tx buffer is
// ready to accept the maximal number of bytes.
extern bool tx_is_empty();

// Try to append a single byte to the tx buffer. Returns true IFF the
// byte was appeneded.
extern bool tx_put_byte(uint8_t byte);


// ----- Wifi module control signals

// Set the state output pin that control the WIFI module. When is_reset
// is true, the WIFI module is help in reset state.
extern void wifi_module_control(bool is_reset);

// Test if the WIFI module signals that incoming data is pending from an
// open connection.
extern bool wifi_module_pending_stream_data();

}  // namespace wifi_io

#endif  // WIFI_WIFI_IO_H

