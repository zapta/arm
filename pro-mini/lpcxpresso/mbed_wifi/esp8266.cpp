// Implementation of esp8266.h.
//
// This implementation assumes that the conn.lua program is running on the
// ESP8266.
//
// ESP8266/ARM_PRO_MINI connections
// --------------------------------
// UTXD -> ARP PRO MINI #16 (RX)
// URXD -> ARP PRO MINI #18 (TX)
// -------------------------------

#include "esp8266.h"

#include <string.h>
#include <ctype.h>

#include "byte_fifo.h"
#include "debug.h"
#include "mbed.h"

namespace esp8266 {

// Invariant: when is_connected is true, connection_id is non zero.
static uint32_t connection_id = 0;
static bool is_connected = false;

// ESP8266 reset output. Active low.
//static DigitalOut wifi_reset(P0_23, 1);

// Hardware serial to wifi device.
static Serial wifi_serial(P0_19, P0_18);  // tx, rx

// Buffer for downstream data bytes. These are binary bytes from
// the down stream.
static uint8_t rx_fifo_buffer[20];
ByteFifo rx_fifo(rx_fifo_buffer, sizeof(rx_fifo_buffer));

// Should be large enough for the largest message we send.
static uint8_t tx_fifo_buffer[200];
ByteFifo tx_fifo(tx_fifo_buffer, sizeof(tx_fifo_buffer));

static Timer tx_timer;

static bool tx_in_progress;

// Reads the ESP8266 and extract the tag lines.
namespace wifi_reader {
enum State {
  // At the beginning of the line.
  IDLE,
  // Not a tag line.
  IGNORED_LINE,
  // Reading a tag line (starts with '!' or '+').
  READING_TAG_LINE,
  // A tag line is available in the line buffer.
  TAG_LINE_READY,
};

static State state;
// Line buffer is managed as a null terminated string.
static unsigned int tag_line_size;
static char tag_line_buffer[20];

bool inline is_line_buffer_full() {
  // -1 because it is managed as a null terminated string.
  return tag_line_size >= (sizeof(tag_line_buffer) - 1);
}

// Return true if ok, otherwise buffer was full.
inline bool append_to_line_buffer(const char c) {
  if (is_line_buffer_full()) {
    return false;
  }
  tag_line_buffer[tag_line_size++] = c;
  tag_line_buffer[tag_line_size] = '\0';
  return true;
}

// Get ready to the next line.
void next_tag_line() {
  state = IDLE;
  tag_line_size = 0;
  tag_line_buffer[0] = '\0';
}

void flush_input() {
  int n = 0;
  while (wifi_serial.readable()) {
    wifi_serial.getc();
    n++;
  }
  debug.printf("*** Flushed %d input chars\n", n);
  rx_fifo.reset();
  next_tag_line();
}

void setup() {
  flush_input();
}

void loop() {
  while (state != TAG_LINE_READY && wifi_serial.readable()) {
    const char c = wifi_serial.getc();

    if (c == '\r') {
      debug.printf("{CR}\n");
    } else if (c == '\n') {
      debug.printf("{LF}\n");
    } else {
      debug.printf("{%c}\n", c);
    }

    switch (state) {
      case IDLE:
        if (c == '>' || c == ' ' || c == '\r' || c == '\n') {
          continue;
        }
        if (c == '!' || c == '+') {
          state = READING_TAG_LINE;
          append_to_line_buffer(c);
          continue;
        }
        state = IGNORED_LINE;
        continue;

      case IGNORED_LINE:
        if (c == '\n') {
          next_tag_line();
        }
        continue;

      case READING_TAG_LINE:
        if (c == '\r') {
        } else if (c == '\n') {
          state = TAG_LINE_READY;
        } else {
          append_to_line_buffer(c);
        }
        continue;

      case TAG_LINE_READY:
        return;

      default:
        debug.printf("* Unknown state:%d\n", state);
        next_tag_line();
    }
  }
}
}  // namespace wifi_reader

static void resetConnectionState() {
  tx_timer.start();
  tx_in_progress = 0;
  tx_fifo.reset();
  rx_fifo.reset();
}


void setup() {
  // TODO: reset the ESP8266 via its reset pin.

  //wifi_serial.baud(115200);
  wifi_serial.baud(9600);
  debug.printf("*** 9600\n");

  wifi_serial.format(8, SerialBase::None, 1);
  wifi_reader::setup();
  connection_id = 0;
  is_connected = false;

  resetConnectionState();

}

// Returns 0-15 for a lower case hex char, or -1 otherwise.
static int hexCharToInt(const char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f') {
    return (c - 'a') + 10;
  }
  return -1;
}


static void rx_loop() {
  wifi_reader::loop();

  if (wifi_reader::state != wifi_reader::TAG_LINE_READY) {
    return;
  }

  //debug.printf("TL [%s]\n", wifi_reader::tag_line_buffer);

  const char tag = wifi_reader::tag_line_buffer[0];

  // Handle connection report.
  if (tag == '!') {
    const char flag = wifi_reader::tag_line_buffer[1];
    if (flag != '0' && flag != '1') {
      debug.printf("BAD [%s]\n", wifi_reader::tag_line_buffer);
    } else if (flag == '1' && !is_connected) {
      // New connection.
      is_connected = true;
      // NOTE: not bothering with 32 bits wrap around to zero that will
      // break the assumption that connection_id is non zero when is_connected
      // is true.
      connection_id++;
      resetConnectionState();
      //rx_fifo.reset();
    } else if (flag == '0' && is_connected) {
      // Connection lost.
      is_connected = false;
      //rx_fifo.reset();
      resetConnectionState();
    }
    wifi_reader::next_tag_line();
    return;
  }

  // Handle rx byte (only if connected)
  if (tag == '+') {
    if (is_connected) {
      // -1 if not in [0-9a-f].
      const int hex1 = hexCharToInt(wifi_reader::tag_line_buffer[1]);
      const int hex2 = hexCharToInt(wifi_reader::tag_line_buffer[2]);
      if (hex1 >= 0 && hex2 >= 0) {
        const uint8_t b = (hex1 << 4) + hex2;
        debug.printf("RX[%02x]\n", b);
        if (!rx_fifo.putByte(b)) {
          debug.printf("RX FULL\n");
        }
      } else {
        // TODO: trigger protcol panic and reconnection
        debug.printf("BAD LN: [%s]\n", wifi_reader::tag_line_buffer);
      }
    }
    wifi_reader::next_tag_line();
    return;
  }

  debug.printf("??: [%s]\n", wifi_reader::tag_line_buffer);
  wifi_reader::next_tag_line();
}

static void tx_loop() {
  if (!tx_fifo.size()) {
    if (tx_in_progress) {
      if (tx_timer.read_ms() > 1000) {
        tx_timer.reset();
        debug.puts("FLUSH()\n");
        wifi_serial.puts("FLUSH()\n");
        tx_in_progress = false;
      }
    }
    return;
  }

  if (tx_timer.read_ms() < 1000) {
    return;  // wait
  }

  wifi_serial.puts("TX(\"");
  debug.puts("TX(\"");
  for (int i = 0; i < 20; i++) {
    uint8_t b;
    if (!tx_fifo.getByte(&b)) {
      break;
    }
    debug.printf("\\%u", b);
    wifi_serial.printf("\\%u", b);
  }

    wifi_serial.puts("\")\n");
    debug.puts("\")\n");
    tx_in_progress = true;
    tx_timer.reset();

}

void loop() {
  rx_loop();
  tx_loop();
}

uint32_t connectionId() {
  // NOTE: if is_connected is true then connection_id is guaranteed to
  // be non zero.
  return is_connected ? connection_id : 0;
}


void reconnect() {
  debug.printf("*** RECONNECT ***\n");
  wifi_serial.puts("if sock then sock:close() end\n");
}

void dumpState() {
  debug.printf("### is_conn=%d, reader=%d.%d[%s], rx=%d, tx=%d\n", is_connected,
      wifi_reader::state, wifi_reader::tag_line_size,
      wifi_reader::tag_line_buffer, rx_fifo.size(), tx_in_progress);
}

}  // namespace esp8266
