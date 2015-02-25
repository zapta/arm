#include "mbed.h"
#include "USBSerial.h"
#include <string.h>

// This file is not checked in git. It contains the following
// definitions (adapt to your own access point):
// #define WIFI_CREDENTIALS_AP_SSID "xxx"
// #define WIFI_CREDENTIALS_AP_PASSWORD "yyy"
//
#include "_gitignore_wifi_credentials.h"

static const char AT_AP_CONNECT[] =
    "AT+CWJAP=\""
    WIFI_CREDENTIALS_AP_SSID
    "\",\""
    WIFI_CREDENTIALS_AP_PASSWORD
    "\"";

static const char AT_AP_LIST[] =
    "AT+CWLAP";

static const char* kConnCommands[] = {
    "AT",
    "AT+CWMODE=3",
    //"AT+RST",
    //"AT",
    AT_AP_CONNECT,
    "AT+CIFSR",
    "AT+CIPMUX=1",
    "AT+CIPSERVER=1,80",
    NULL
};

// Initial delay after reset before communicating with the
// ESP8266.
static const int kInitialDelayMillis = 1000;

static DigitalOut led(P0_7);  // LED pin on my board.

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

static inline void Chip_Clock_DisablePeriphClock()
{
  LPC_SYSCON->SYSAHBCLKCTRL &= ~(1 << (15));
}

void Chip_WWDT_DeInit()
{
  //LPC_SYSCON->SYSAHBCLKCTRL;
  Chip_Clock_DisablePeriphClock();
}

#define IAP_ENTRY_LOCATION  0X1FFF1FF1

typedef void (*IAP_ENTRY_T)(unsigned int[], unsigned int[]);

static inline void iap_entry(unsigned int cmd_param[], unsigned int status_result[])
{
  ((IAP_ENTRY_T) IAP_ENTRY_LOCATION)(cmd_param, status_result);
}

void ReinvokeISP()
{
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
  __set_MSP(*((uint32_t *)0x00000000));

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


class CommandExecutor{
public:
  enum State {
    IDLE,
    RECIEVING,
    DONE_OK,
    DONE_ERROR,
    DONE_TIMEOUT,
  };

  CommandExecutor() {
    _state = IDLE;
    _timer.start();
    reset_line_buffer();
    //_buffer_char_count = 0;
  }

  // Should be called on each iteration of main loop.
  void loop() {
    if (_state != RECIEVING) {
      return;
    }

    // Here when in receiving state.

    // Handle timeout.
    // TODO: allow per command timeout.
    if (_timer.read_ms() > 10*1000) {
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
        _line_buffer[_buffer_char_count++]  = c;
        _line_buffer[_buffer_char_count]  = '\0';
      } else {
        //usb_serial.printf("* normal [%c] FULL\n", c);
      }
      return;
    }

    // Here when the char is LF which indicates end of line.

    usb_serial.printf("* RESP [%s]\n", _line_buffer);

    // Handle OK status.
    if (strcmp(_line_buffer, "OK") == 0 || strcmp(_line_buffer, "no change") == 0) {
      usb_serial.printf("* OK (%d ms)\n", _timer.read_ms());
      _state = DONE_OK;
      return;
    }

    // Handle error status.
    if (strcmp(_line_buffer, "ERROR") == 0) {
      usb_serial.printf("* ERR (%d ms)\n", _timer.read_ms());
      _state = DONE_ERROR;
      return;
    }

    // Here it's a generic line that doesn't indicate command status. We
    // simply ignore and will process the next line.
    reset_line_buffer();
    //usb_serial.printf("* NEW_LINE\n");
  }

  void start_command(const char* cmd, int timeout_ms) {
    usb_serial.printf("* cmd [%s]\n", cmd);
    wifi_serial.puts(cmd);
    wifi_serial.puts("\r\n");
    reset_line_buffer();
    _timer.reset();
    _state = RECIEVING;
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
  Timer _timer;
  unsigned int _buffer_char_count;
  char _line_buffer[50];

  void reset_line_buffer() {
    //memset(_line_buffer, '\0', sizeof(_line_buffer));
    _line_buffer[0] = '\0';
    _buffer_char_count = 0;
  }
};

class CommandSequenceExecutor  {
public:
  enum SeqState {
    SEQ_IDLE,
    SEQ_EXECUTING,
    SEQ_DONE_OK,
    SEQ_DONE_ERR
  };

  CommandSequenceExecutor() {
    _state = SEQ_IDLE;
    _current_cmd = NULL;
  }



  void start_sequence(const char** command_list) {
    _current_cmd = command_list;
    start_next_command();
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
      _current_cmd = NULL;
      return;
    }

    start_next_command();
  }

  // Is current state is 'in progress'?
   bool is_seq_done() {
     return _state == SEQ_DONE_OK || _state == SEQ_DONE_ERR;
   }

   bool is_command_done_ok() {
     return _state == SEQ_DONE_OK;
   }

private:
  CommandExecutor _command_exec;
  SeqState _state;
  const char** _current_cmd;

  // If the list is empty we treat it as an error.
  void start_next_command() {
     if (*_current_cmd == NULL) {
       usb_serial.printf("# SEQ OK\n");
       _state = SEQ_DONE_OK;
       _current_cmd = NULL;
       return;
     }

     _state = SEQ_EXECUTING;
     usb_serial.printf("# SEQ [%s]\n", *_current_cmd);
     _command_exec.start_command(*_current_cmd, 8000);
     _current_cmd++;
   }
};

static Timer timer;

static CommandSequenceExecutor seq;


void setup() {
  timer.start();
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
      seq.start_sequence(kConnCommands);
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
    usb_serial.printf("[%c %02x]\n", c, c);
  }
}

int main(void) {
  setup();
  for(;;) {
    loop();
  }
}
