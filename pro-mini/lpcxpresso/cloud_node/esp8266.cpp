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

//#include "byte_fifo.h"
#include "debug.h"
#include "mbed.h"
#include "passive_timer.h"

namespace esp8266 {

// Invariant: when is_connected is true, connection_id is non zero.
static uint32_t connection_id = 0;
static bool is_connected = false;

// ESP8266 reset output. Active low.
//static DigitalOut wifi_reset(P0_23, 1);

// TODO: revisit size.
static uint8_t serial_rx_irq_fifo_buffer[100];
ByteFifoRxIrq serial_rx_irq_fifo(serial_rx_irq_fifo_buffer, sizeof(serial_rx_irq_fifo_buffer));

// Hardware serial to wifi device.
static Serial wifi_serial(P0_19, P0_18);  // tx, rx

static void Rx_interrupt() {
  while ((wifi_serial.readable())) {
    // TODO: hadd a 'was full' flag in the fifo so we can detect missing
    // bytes.
    serial_rx_irq_fifo.putByteInIrq(wifi_serial.getc());
  }
}

// Buffer for downstream data bytes. These are binary bytes from
// the down stream.
//
// TODO: determine the proper size. Assumption is that it should
// be able to contain the data of a long (~15-20ms) drawing operation
// but not sure what is the internal buffer size of the serial driver.
// Can also call rx polling from the drawing loop. Should be
// further inverstigated.
//
static uint8_t rx_fifo_buffer[30];
ByteFifo rx_fifo(rx_fifo_buffer, sizeof(rx_fifo_buffer));

// Should be large enough to contain the largest message we send
// since we send it in a single polling loop. If this is too much
// memory, may try to break the sending into multiple polling
// iterations.
//
static uint8_t tx_fifo_buffer[200];
ByteFifo tx_fifo(tx_fifo_buffer, sizeof(tx_fifo_buffer));
//FifoTx tx_fifo;

static PassiveTimer tx_timer;

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

bool inline isLineBufferFull() {
  // -1 because it is managed as a null terminated string.
  return tag_line_size >= (sizeof(tag_line_buffer) - 1);
}

// Return true if ok, otherwise buffer was full.
inline bool appendToLineBuffer(const char c) {
  if (isLineBufferFull()) {
    return false;
  }
  tag_line_buffer[tag_line_size++] = c;
  tag_line_buffer[tag_line_size] = '\0';
  return true;
}

// Get ready to the next line.
void nextTagLine() {
  state = IDLE;
  tag_line_size = 0;
  tag_line_buffer[0] = '\0';
}

void flushInput() {
  int n = 0;
  while (wifi_serial.readable()) {
    wifi_serial.getc();
    n++;
  }
  debug.printf("*** Flushed %d input chars\n", n);
  rx_fifo.reset();
  nextTagLine();
}

void initialize() {
  flushInput();
}

void polling() {
  while (state != TAG_LINE_READY ) {
    //&& wifi_serial.readable()) {
  //}
    //const char c = wifi_serial.getc();
    uint8_t c = 0;
    if (!serial_rx_irq_fifo.getByte(&c)) {
      return;
    }

    // Enable to dump all communication from esp8266 Lua (verbose).
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
          appendToLineBuffer(c);
          continue;
        }
        state = IGNORED_LINE;
        continue;

      case IGNORED_LINE:
        if (c == '\n') {
          nextTagLine();
        }
        continue;

      case READING_TAG_LINE:
        if (c == '\r') {
        } else if (c == '\n') {
          state = TAG_LINE_READY;
        } else {
          appendToLineBuffer(c);
        }
        continue;

      case TAG_LINE_READY:
        return;

      default:
        debug.printf("* Unknown state:%d\n", state);
        nextTagLine();
    }
  }
}
}  // namespace wifi_reader

static void resetForANewConnection() {
  tx_timer.reset();
  tx_in_progress = 0;
  tx_fifo.reset();
  rx_fifo.reset();
}

void initialize() {
  // TODO: reset the ESP8266 using its reset pin.
  wifi_serial.baud(9600);
  debug.printf("*** 9600\n");
  wifi_serial.format(8, SerialBase::None, 1);
  wifi_serial.attach(&Rx_interrupt, Serial::RxIrq);

  wifi_reader::initialize();
  connection_id = 0;
  is_connected = false;
  resetForANewConnection();
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

static void rx_polling() {
  wifi_reader::polling();

  if (wifi_reader::state != wifi_reader::TAG_LINE_READY) {
    return;
  }

  // Enable to dump tag lines recieved from the esp8266 Lua.
  debug.printf("tag: [%s]\n", wifi_reader::tag_line_buffer);

  const char tag = wifi_reader::tag_line_buffer[0];

  // Handle connection report.
  // "!0" -> not connected
  // "!1" -> connected
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
      resetForANewConnection();
      //rx_fifo.reset();
    } else if (flag == '0' && is_connected) {
      // Connection lost.
      is_connected = false;
      //rx_fifo.reset();
      resetForANewConnection();
    }
    wifi_reader::nextTagLine();
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
        //debug.printf("RX[%02x]\n", b);
        if (!rx_fifo.putByte(b)) {
          // TODO: propogate it somehow to protocl panic. For example, has a was_full
          // flag in the fifo that protocol_rx will poll.
          debug.printf("RX FULL\n");
        }
      } else {
        // TODO: trigger protcol panic and reconnection
        debug.printf("BAD LN: [%s]\n", wifi_reader::tag_line_buffer);
      }
    }
    wifi_reader::nextTagLine();
    return;
  }

  debug.printf("??: [%s]\n", wifi_reader::tag_line_buffer);
  wifi_reader::nextTagLine();
}

static void tx_polling() {
  if (!tx_fifo.size()) {
    // Done sending bytes to Lua, issue a FLUSH
    if (tx_in_progress) {
      // TODO: define a const for time SEND and FLUSH.
      if (tx_timer.usecs() >= 300000) {
        tx_timer.reset();
        debug.puts("FLUSH()\n");
        wifi_serial.puts("FLUSH()\n");
        tx_in_progress = false;
      }
    }
    return;
  }

  // Here when having pending bytes. Have sufficient time gap
  // before sending next line.
  // TODO: define time const.
  if (tx_timer.usecs() < 300000) {
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

void polling() {
  rx_polling();
  tx_polling();
}

uint32_t connectionId() {
  // NOTE: if is_connected is true then connection_id is guaranteed to
  // be non zero.
  return is_connected ? connection_id : 0;
}

void abortCurrentConnection() {
  debug.printf("*** RECONNECT ***\n");
  wifi_serial.puts("if sock then sock:close() end\n");
}

void dumpInternalState() {
  debug.printf("esp8266 is_conn=%d, reader=%d.%d[%s], rx=%d, tx=%d\n",
      is_connected, wifi_reader::state, wifi_reader::tag_line_size,
      wifi_reader::tag_line_buffer, rx_fifo.size(), tx_in_progress);
}

}  // namespace esp8266
