#include "mbed.h"
#include "USBSerial.h"
#include <string.h>
#include <ctype.h>

// This file is not checked in git. It contains the following
// definitions (adapt to your own access point):
// #define WIFI_CREDENTIALS_AP_SSID "xxx"
// #define WIFI_CREDENTIALS_AP_PASSWORD "yyy"
//
#include "_gitignore_wifi_credentials.h"

static const char AT_AP_CONNECT[] = "AT+CWJAP=\""
WIFI_CREDENTIALS_AP_SSID
"\",\""
WIFI_CREDENTIALS_AP_PASSWORD
"\"";

static const char AT_AP_LIST[] = "AT+CWLAP";

static const char* kOkStatusResponses[] = {
    "OK",
    "ready",
    "no change",
    "SEND OK",
    NULL, };

static const char* kErrorStatusResponses[] = {
    "ERROR",
    "Fail",
    NULL, };

// Null terminated list.
static bool isStringInList(const char* str, const char** list) {
  while (*list) {
    if (strcmp(str, *list++) == 0) {
      return true;
    }
  }
  return false;
}

class Action {
public:
  enum Type {
    RESET_PULSE,
    DELAY,
    FLUSH_INPUT,
    AT_COMMAND,
    END,
  };

  const uint8_t type;
  const char* at_command;
  const uint16_t timeout_ms;
};

static const Action kInitSequence[] = {
    {
        Action::RESET_PULSE,
        NULL,
        50, },
    {
        Action::DELAY,
        NULL,
        1000 },
    {
        Action::FLUSH_INPUT,
        NULL,
        0 },
    {
        Action::AT_COMMAND,
        "AT",
        500 },
    {
        Action::AT_COMMAND,
        "AT+CWMODE=3",
        1000 },
    {
        Action::AT_COMMAND,
        AT_AP_CONNECT,
        10000 },
    {
        Action::AT_COMMAND,
        "AT+CIFSR",
        500 },
    {
        Action::AT_COMMAND,
        "AT+CIPMUX=1",
        500 },
    {
        Action::AT_COMMAND,
        "AT+CIPSERVER=1,80",
        1000 },
    {
        Action::END,
        NULL,
        0 }, };

// Initial delay after reset before communicating with the
// ESP8266.
static const int kInitialDelayMillis = 1000;

//static DigitalOut led(P0_7);  // LED pin on my board.
static DigitalOut led(P0_20, 0);  // LED pin on my board.

// ESP8266 reset output. Active low.
static DigitalOut wifi_reset(P0_23, 1);

// Virtual serial port over USB.
// Using default USBSerial device parameters except for 'flase' for
// non blocking if USB is not connected (e.g. when running on external
// power supply).
static USBSerial usb_serial(0x1f00, 0x2012, 0x0001, false);

// Hardware serial to wifi device.
static Serial wifi_serial(P0_19, P0_18);  // tx, rx

// TODO: verify correctness of ISP monitor code.
// TODO: move ISP monitor to a seperate file.

static DigitalIn isp_button(P0_1);

//void setup() {
//  Chip_GPIO_SetPinDIRInput(LPC_GPIO, ISP_BUTTON_PORT, ISP_BUTTON_BIT);
//}

// This data must be global because we access it after changing the stack
// for the ROM entry point.
static unsigned int command[5];
static unsigned int result[4];

#define init_msdstate() *((uint32_t *)(0x10000054)) = 0x0

static inline void Chip_Clock_DisablePeriphClock() {
  LPC_SYSCON->SYSAHBCLKCTRL &= ~(1 << (15));
}

void Chip_WWDT_DeInit() {
  //LPC_SYSCON->SYSAHBCLKCTRL;
  Chip_Clock_DisablePeriphClock();
}

#define IAP_ENTRY_LOCATION  0X1FFF1FF1

typedef void (*IAP_ENTRY_T)(unsigned int[], unsigned int[]);

static inline void iap_entry(unsigned int cmd_param[],
    unsigned int status_result[]) {
  ((IAP_ENTRY_T) IAP_ENTRY_LOCATION)(cmd_param, status_result);
}

void ReinvokeISP() {
  // Disable SYSTICK timer and interrupt before calling into ISP */
  SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);

  //NVIC_DisableIRQ(USB_IRQn)
  // Disable USB interrupts.
  //NVIC_DisableIRQ(USB0_IRQn);
  NVIC_DisableIRQ(USB_IRQn);

//LPC_S
  // LPC_SYSCON->SYSAHBCLKCTRL |= 0x04000;
//  /* make sure USB clock is turned on before calling ISP */
//   LPC_SYSCTL->SYSAHBCLKCTRL |= 0x04000;
//   /* make sure 32-bit Timer 1 is turned on before calling ISP */
//   LPC_SYSCTL->SYSAHBCLKCTRL |= 0x00400;
//   /* make sure GPIO clock is turned on before calling ISP */
//   LPC_SYSCTL->SYSAHBCLKCTRL |= 0x00040;
//   /* make sure IO configuration clock is turned on before calling ISP */
//   LPC_SYSCTL->SYSAHBCLKCTRL |= 0x10000;

  /* make sure USB clock is turned on before calling ISP */
  LPC_SYSCON->SYSAHBCLKCTRL |= 0x04000;
  /* make sure 32-bit Timer 1 is turned on before calling ISP */
  LPC_SYSCON->SYSAHBCLKCTRL |= 0x00400;
  /* make sure GPIO clock is turned on before calling ISP */
  LPC_SYSCON->SYSAHBCLKCTRL |= 0x00040;
  /* make sure IO configuration clock is turned on before calling ISP */
  LPC_SYSCON->SYSAHBCLKCTRL |= 0x10000;

  /* make sure AHB clock divider is 1:1 */
  LPC_SYSCON->SYSAHBCLKDIV = 1;

  // TODO: test this with an actual active WDT. (do we need it at all?
  // the ROM functionality may disable the WDT anyway).

  //Chip_WWDT_DeInit(LPC_WWDT);
  Chip_WWDT_DeInit();

  // Send Reinvoke ISP command to ISP entry point.
  command[0] = 57;

  // Initialize Storage state machine.
  // TODO: what does this mean? Do we need it?
  init_msdstate();

  // Set stack pointer to ROM value (reset default) This must be the last
  // piece of code executed before calling ISP, because most C expressions
  // and function returns will fail after the stack pointer is changed. */
  __set_MSP(*((uint32_t *) 0x00000000));

  // Enter ISP. We call "iap_entry" to enter ISP because the ISP entry is done
  // through the same command interface as IAP. */
  // xiap_entry(command, result);
  iap_entry(command, result);
  // Not supposed to come back!
}

void isp_loop() {
  if (!isp_button.read()) {
    usb_serial.printf("\n**GOING ISP**\n\n");
    wait_ms(500);
    ReinvokeISP();
  }
}

//class WifiLineReader {
//  enum State {
//    READING_LINE,
//    READING_DATA_SIZE,
//    READING_DATA,
//    LINE_OK,
//    ERROR,
//  };
//
//  WifiLineReader() {
//    reset(true);
//  }
//
//  void reset(bool flush_input) {
//    _state = READING_LINE;
//    _line_size = 0;
//    _data_size = 0;
//    _expected_data_size = 0;
//
//    if (flush_input) {
//      while (wifi_serial.readable()) {
//        wifi_serial.getc();
//      }
//    }
//  }
//
//  void loop() {
//    //@@@ TODO: detect timeout within a line.
//    if (!wifi_serial.readable()) {
//         return;
//    }
//    switch (_state) {
//    case READING_LINE:
//      loop_reading_line();
//      break;
//    case READING_DATA_SIZE:
//      loop_reading_data_size();
//      break;
//    case READING_DATA:
//      break;
//    case LINE_OK:
//      break;
//    case ERROR:
//      break;
//    default:
//      _state = ERROR;
//      break;
//    }
//  }
//
//  void loop_reading_line() {
//     const char c = wifi_serial.getc();
//     // Ignore CR.
//     if (c == '\r') {
//       return;
//     }
//
//     // Handle LF (end of line)
//     if (c == '\n') {
//        _state = LINE_OK;
//       return;
//     }
//
//     // Append to line buffer. Error if full.
//     if (!append_to_line_buffer(c)) {
//       _state = ERROR;
//       return;
//     }
//
//     // If next is the data size field of the IPD response (incoming data) then
//     // switch state to read it. Doing it only when the size if right to avoid
//     // too many string comparisons.
//     if (_line_size == 7 && strcmp(_line_buffer, "+IPD,0,") == 0) {
//       _state = READING_DATA_SIZE;
//     }
//   }
//
//  void loop_reading_data_size() {
//    const char c = wifi_serial.getc();
//    if (!append_to_line_buffer(c)) {
//      _state = ERROR;
//      return;
//    }
//
//    // Detect field terminator.
//    if (c == ':') {
//      _state = (_expected_data_size > 0) ? READING_DATA : READING_LINE;
//      return;
//    }
//
//    // Expecting a decimal digit. Updated the expected size.
//    const int digit = (c - '0');
//    if (digit < 0 || digit > 9) {
//      _state = ERROR;
//      return;
//    }
//    _expected_data_size = (_expected_data_size * 10) + digit;
//    // @@@ TODO: if expected data size larger than data buffer size
//    // set state to error.
//  }
//
//  void loop_reading_data() {
//    const char c = wifi_serial.getc();
//    if (is_data_buffer_full()) {
//      _state = ERROR;
//      return;
//    }
//    _data_buffer[_data_size++] = c;
//    if (_data_size >= _expected_data_size) {
//      // Switch to read the rest of the file. Expecting here only the terminating
//      // CR/LF.
//      _state = READING_LINE;
//    }
//  }
//
//
//  // Return true if ok, otherwise buffer was full.
//  inline bool append_to_line_buffer(const char c) {
//    if (is_line_buffer_full()) {
//      return false;
//    }
//    _line_buffer[_line_size++] = c;
//    _line_buffer[_line_size] = '\0';
//    return true;
//  }
//
//
//  bool inline is_line_buffer_full() {
//    // -1 because it is managed as a null terminated string.
//    return _line_size >= (sizeof(_line_buffer) - 1);
//  }
//
//  bool inline is_data_buffer_full() {
//    // Straight binary data, no null terminator.
//    return _data_size >= (sizeof(_data_buffer));
//  }
//
//  State _state;
//
//  // Line buffer is managed as a null terminated string.
//  unsigned int _line_size;
//  char _line_buffer[30 + 1];
//
//  // Data buffer is managed as a straight binary data, no null terminator.
//  unsigned int _expected_data_size;
//  unsigned int _data_size;
//  uint8_t _data_buffer[256];
//};

class WifiActionExecutor {
public:
  enum State {
    IDLE,
    IN_PROGRESS,
    DONE_OK,
    DONE_ERROR,
    DONE_TIMEOUT,
  };

  WifiActionExecutor() {
    _state = IDLE;
    _action = NULL;
    _timer.start();
    reset_line_buffer();
  }

  // Should be called on each iteration of main loop.
  void loop() {
    if (_state != IN_PROGRESS) {
      return;
    }
    switch (_action->type) {
      case Action::RESET_PULSE:
        if (_timer.read_ms() >= _action->timeout_ms) {
          wifi_reset.write(1);
          _state = DONE_OK;
          usb_serial.printf("* DONE OK (%d ms)\n", _timer.read_ms());
        }
        break;

      case Action::DELAY:
        if (_timer.read_ms() >= _action->timeout_ms) {
          _state = DONE_OK;
          usb_serial.printf("* DONE OK (%d ms)\n", _timer.read_ms());
        }
        break;

      case Action::AT_COMMAND:
        at_command_loop();
        break;

      case Action::FLUSH_INPUT:
      case Action::END:
      default:
        usb_serial.printf("* LOOP: unexpected action %d\n", _action->type);
        _state = DONE_ERROR;
        break;
    }
  }

  // make private
  void at_command_loop() {
    // Handle timeout.
    // TODO: allow per command timeout.
    if (_timer.read_ms() > _action->timeout_ms) {
      usb_serial.printf("* TIMEOUT (%d ms)\n", _timer.read_ms());
      _state = DONE_TIMEOUT;
      return;
    }

    // Do nothing if no wifi char.
    if (!wifi_serial.readable()) {
      return;
    }

    const char c = wifi_serial.getc();
    //usb_serial.printf("--%02x\n", c);

    // Ignore CR.
    if (c == '\r') {
      //usb_serial.printf("* ignore CR\n");
      return;
    }

    // Handle normal chars.
    if (c != '\n') {
      // usb_serial.printf("* normal [%c]\n", c);
      if (_buffer_char_count < (sizeof(_line_buffer) - 1)) {
        // Append char.
        //usb_serial.printf("* normal [%c]\n", c);
        _line_buffer[_buffer_char_count++] = c;
        _line_buffer[_buffer_char_count] = '\0';
      } else {
        //usb_serial.printf("* normal [%c] FULL\n", c);
      }
      return;
    }

    // Here when the char is LF which indicates end of line.

    usb_serial.printf("* RESP [%s]\n", _line_buffer);

    // Handle OK status.
    if (isStringInList(_line_buffer, kOkStatusResponses)) {
      //if (strcmp(_line_buffer, "OK") == 0 || strcmp(_line_buffer, "no change") == 0) {
      usb_serial.printf("* OK (%d ms)\n", _timer.read_ms());
      _state = DONE_OK;
      return;
    }

    // Handle error status.
    //if (strcmp(_line_buffer, "ERROR") == 0) {
    if (isStringInList(_line_buffer, kErrorStatusResponses)) {
      usb_serial.printf("* ERR (%d ms)\n", _timer.read_ms());
      _state = DONE_ERROR;
      return;
    }

    // Here it's a generic line that doesn't indicate command status. We
    // simply ignore and will process the next line.
    reset_line_buffer();
    //usb_serial.printf("* NEW_LINE\n");
  }

  void start_action(const Action* action) {
    // Common initialization.
    //usb_serial.printf("* Action: type=%d\n", action->type);
    _action = action;
    _timer.reset();
    _state = IN_PROGRESS;
    reset_line_buffer();

    // Action type specific.
    switch (action->type) {
      case Action::RESET_PULSE:
        usb_serial.printf("\n* ACTION: RESET [%d]\n", action->timeout_ms);
        wifi_reset.write(0);
        // Will end by timer.
        break;

      case Action::DELAY:
        usb_serial.printf("\n* ACTION: DELAY [%d]\n", action->timeout_ms);
        // Nothing to do. Will end by timer.
        break;

      case Action::FLUSH_INPUT: {
        usb_serial.printf("\n* ACTION: FLUSH_INPUT\n");
        int n = 0;
        while (wifi_serial.readable()) {
          wifi_serial.getc();
          n++;
        }
        _state = DONE_OK;
        usb_serial.printf("* DONE OK (%d ms, %d chars)\n", _timer.read_ms(), n);
      }
        break;

      case Action::AT_COMMAND:
        usb_serial.printf("\n* ACTION: CMD [%s]\n", action->at_command);
        wifi_serial.puts(action->at_command);
        wifi_serial.puts("\r\n");
        break;

        // The sequence executor is expected to detect the END action and stop
        // before calling this.
      case Action::END:
      default:
        usb_serial.printf("\n* ACTION: unexpected type %d\n", action->type);
        _state = DONE_ERROR;
        break;
    }
  }

  // Is current state is 'in progress'?
  bool is_command_done() {
    return _state == DONE_OK || _state == DONE_ERROR || _state == DONE_TIMEOUT;
  }

  bool is_command_done_ok() {
    return _state == DONE_OK;
  }

private:
  State _state;
  const Action* _action;
  Timer _timer;
  unsigned int _buffer_char_count;
  char _line_buffer[50];

  void reset_line_buffer() {
    //memset(_line_buffer, '\0', sizeof(_line_buffer));
    _line_buffer[0] = '\0';
    _buffer_char_count = 0;
  }
};

class CommandSequenceExecutor {
public:
  enum SeqState {
    SEQ_IDLE,
    SEQ_EXECUTING,
    SEQ_DONE_OK,
    SEQ_DONE_ERR
  };

  CommandSequenceExecutor() {
    _state = SEQ_IDLE;
    _current_action = NULL;
  }

  void start_sequence(const Action* action_list) {
    _current_action = action_list;
    start_next_action();
  }

  void loop() {
    _command_exec.loop();

    if (_state != SEQ_EXECUTING) {
      return;
    }

    if (!_command_exec.is_command_done()) {
      return;
    }

    if (!_command_exec.is_command_done_ok()) {
      _state = SEQ_DONE_ERR;
      usb_serial.printf("# SEQ ERR\n");
      _current_action = NULL;
      return;
    }

    start_next_action();
  }

  // Is current state is 'in progress'?
  bool is_seq_done() {
    return _state == SEQ_DONE_OK || _state == SEQ_DONE_ERR;
  }

  bool is_command_done_ok() {
    return _state == SEQ_DONE_OK;
  }

private:
  WifiActionExecutor _command_exec;
  SeqState _state;
  const Action* _current_action;

  // If the list is empty we treat it as an error.
  void start_next_action() {
    if (_current_action->type == Action::END) {
      usb_serial.printf("\n# SEQ OK\n");
      _state = SEQ_DONE_OK;
      _current_action = NULL;
      return;
    }

    _state = SEQ_EXECUTING;
    // usb_serial.printf("# SEQ [%s]\n", *_current_cmd);
    _command_exec.start_action(_current_action);
    _current_action++;
  }
};

static Timer timer;

static CommandSequenceExecutor seq;

void setup() {
  timer.start();
  // Set wifi reset high (inactive).
  isp_button.mode(PullUp);
  wifi_serial.baud(9600);
  wifi_serial.format(8, SerialBase::None, 1);
}

enum MainState {
  MAIN_INITIAL_DELAY,
  MAIN_SEQUENCE,
  MAIN_READY,
};

void loop() {
  isp_loop();

  static MainState state = MAIN_INITIAL_DELAY;
  //static bool in_delay = true;

  if (state == MAIN_INITIAL_DELAY) {
    if (timer.read_ms() >= 10000) {
      usb_serial.printf("----\n");
      state = MAIN_SEQUENCE;
      seq.start_sequence(kInitSequence);
      timer.reset();
    }
    return;
  }

  if (state == MAIN_SEQUENCE) {
    seq.loop();
    led.write(timer.read_ms() < 100);
    if (seq.is_seq_done()) {
      state = MAIN_READY;
      timer.reset();
    }
    return;
  }

  // Here when ready
  if (wifi_serial.readable()) {
    const char c = wifi_serial.getc();
    switch (c) {
      case '\r':
        usb_serial.puts("<CR>");
        break;
      case '\n':
        usb_serial.puts("<LF>\n");
        break;
      default:
        if (isprint(c)) {
          usb_serial.putc(c);
        } else {
          usb_serial.printf("<%02x>", c);
        }
    }
//    if (c == '\r') {
//      usb_serial.puts("<CR>");
//    }
//    usb_serial.printf("[%c %02x]\n", c, c);
  }
}

int main(void) {
  setup();
  for (;;) {
    loop();
  }
}
