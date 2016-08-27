#include "wifi/wifi_parser.h"

#include "wifi/wifi.h"
#include "wifi/wifi_io.h"

//#include "util/rgb_io.h"

namespace wifi_parser {

// Indicates how to parse the current response.
static ResponseParsingMode parsing_mode = STD;

static const char* kButtonTable[] = {
  "A0:02:DC:09:A5:EE",
  "F0:27:2D:30:68:68",  // green
};

static const int kButtonTableSize = sizeof(kButtonTable) / sizeof(kButtonTable[0]);

// Last parsing result.
static int buttons_set = 0;

// Temp during parsing.
static int parsed_buttons_set = 0;

enum ReaderState {
  // Doing nothing.
  STATE_IDLE,

  // Expecting a response header line with status and data length. E.g. "R000008".
  // Skipping lines that don't match this pattern.
  STATE_READING_HEADER_LINE,

  // Reading the data that follows the header. Behavior depends on current
  // parsing mode.
  STATE_READING_DATA,

  // Done parsing the response.
  STATE_RESPONSE_READY,
};

static ReaderState state;

// Line buffer is managed as a null terminated string.
// TODO: make this static.
unsigned int line_buffer_size;

static const uint32_t kLineBufferCapacity = 100;

// Make this static
//const uint32_t kBufferCapacity = 100;

// TODO: revist size, trim if possible.
// TODO: make this static
// Having an extra char for the null terminator.
char line_buffer[kLineBufferCapacity + 1];

// Valid only in READER_READING_DATA state.
static int data_bytes_left_to_read;

static bool inline isLineBufferFull() {
  return line_buffer_size >= kLineBufferCapacity;
}

// Return true if ok, otherwise buffer was full.
static inline bool appendToLineBuffer(const char c) {
  if (isLineBufferFull()) {
    return false;
  }
  line_buffer[line_buffer_size++] = c;
  line_buffer[line_buffer_size] = '\0';
  return true;
}

bool isResponseParsingDone() {
  //if (serial_rx_irq_overflow) {
  if (wifi_io::is_rx_overflow()) {
    wifi::protocolPanic("RX irq ovf");
  }
  return (state == STATE_RESPONSE_READY);
}

static void resetLineBuffer() {
  line_buffer_size = 0;
  line_buffer[0] = '\0';
}

// Get ready to reading next response. Should be called just before the command
// is sent to the wifi module.
void startParsingNextResponse(ResponseParsingMode response_parsing_mode) {
  parsing_mode = response_parsing_mode;
  wifi_io::rx_reset();
  resetLineBuffer();
  parsed_buttons_set = 0;
  state = STATE_READING_HEADER_LINE;
}

void initialize() {
  //wifi_stream::stream_rx_fifo.reset();
  //resetForNextResponse();
  resetLineBuffer();
  state = STATE_IDLE;
}

// Called after a line was found and terminating CR/LF where consumed.
// Returns true if found a response header. Otherwise line should be ignore.
static bool processHeaderLine() {
  if (line_buffer_size != 7) {
    return false;
  }

  if (line_buffer[0] != 'R') {
    return false;
  }

  const uint8_t error_code = line_buffer[1] - '0';
  if (error_code > 9) {
    return false;
  }

  // TODO: if error code is not zero, should we reader direct_to_rx_fifo flag?
  
  data_bytes_left_to_read = 0;
  for (int i = 2; i < 7; i++) {
    uint8_t d = line_buffer[i] - '0';
    if (d > 9) {
      return false;
    }
    data_bytes_left_to_read = (data_bytes_left_to_read * 10) + d;
  }

  if (data_bytes_left_to_read >= 2) {
    data_bytes_left_to_read -= 2; // CR/LF already got consumed.
  }

  PRINTF("%s\n", line_buffer);
  if (error_code) {
    wifi::protocolPanic("response error status");
  }
  return true;
}

void polling() {
  uint8_t byte = 0;
  while (wifi_io::rx_read_byte(&byte)) {
    DEBUGF("*%d:%02x\n", state, byte);
    switch (state) {

    case STATE_IDLE:
      // Consume the current char. We are not parsing any response.
      break;

    case STATE_READING_HEADER_LINE:
      // Wifi modules terminates line with CR/LF. We ignore the CR.
      if (byte == '\r') {
        break;
      }

      // Handle normal char
      if (byte != '\n') {
        appendToLineBuffer(byte);
        break;
      }

      // Here when end of line.

      // If not a header line then ignore.
      if (!processHeaderLine()) {
        // Ignore this line.
        resetLineBuffer();
        break;
      }

      // Here when header line read
      resetLineBuffer();
      // NOTE: this cannot happen for the client list command so not bothering clearing
      // the button bitset.
      state = data_bytes_left_to_read ? STATE_READING_DATA : STATE_RESPONSE_READY;
      break;


    case STATE_READING_DATA:
      DEBUGF("#%d, %02x\n", data_bytes_left_to_read, byte);
      //data_bytes_left_to_read--;

      data_bytes_left_to_read--;

      // normal char
      if (byte != '\r' && byte != '\n') {
        appendToLineBuffer(byte);
      }

      // if no end of line then continue
      if (data_bytes_left_to_read && byte != '\n') {
        break;
      }

      // handle line.
      PRINTF("LINE: %s\n", line_buffer);

      if (parsing_mode == CLIENT_LIST && line_buffer[0] == '#') {
        for (int i = 0; i < kButtonTableSize; i++) {
          const char* button_mac = kButtonTable[i];
          PRINTF("MAC: [%s]\n", button_mac);
          if (strncmp(&line_buffer[4], button_mac, strlen(button_mac)) == 0) {
            PRINTF("BUTTON: %d\n", i);
            parsed_buttons_set |= (1 << i);
          }
        }
      }

      if (!data_bytes_left_to_read) {
        if (parsing_mode == CLIENT_LIST) {
          buttons_set = parsed_buttons_set;
          //@@@rgb_io::set(buttons_set ? 0xff0000 : 0x001100);
        }
        state = STATE_RESPONSE_READY;
      } else {
        resetLineBuffer();
      }
      break;

    case STATE_RESPONSE_READY:
      // Consume the char. We already have the response.
      break;


    default:
      dumpInternalState();
      wifi::protocolPanic("Unknown parser state");
      state = STATE_IDLE;
    }
  }
}

int getPressedButtonSet() {
  return buttons_set;
}

void dumpInternalState() {
  PRINTF(
      "PARSER state=%d, mode=%d, size=%d, buttons=%d, buffer=[%s]\n",
      state, parsing_mode, line_buffer_size, buttons_set, line_buffer);
}

} // namespace wifi_parser
