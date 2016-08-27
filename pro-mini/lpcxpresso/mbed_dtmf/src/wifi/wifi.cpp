// Implementation of wifi.h.
//

// TODO: add timeouts to the individual states, protocolPanic if exceedes.
// TODO: protocol panic if rx buffer is full.

#include "wifi/wifi.h"

#include <algorithm>    // for std::min
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <wifi/wifi_parser.h>
//#include <wifi/wifi_streams.h>

//#include "config/config.h"
#include "util/byte_fifo.h"
#include "util/common.h"
#include "util/passive_timer.h"
#include "util/string_util.h"
#include "wifi/wifi_io.h"


namespace wifi {

const static int kWifiStatusPollingIntervalMillis = 1000;

// Invariant: when is_connected is true, connection_id is non zero.
static uint32_t session_id = 0;

static void send_wifi_command(wifi_parser::ResponseParsingMode response_parsing_mode, const char* s) {
  wifi_parser::startParsingNextResponse(response_parsing_mode);
  PRINT(s);
  while (*s) {
    if (!wifi_io::tx_put_byte(*s++)) {
      protocolPanic("cmd tx overflow");
      return;
    }
  }
}

static void send_wifi_commandf(wifi_parser::ResponseParsingMode response_parsing_mode, const char* format, ...) {
  // Assuming single thread, using static buffer.
  static char buffer[100];
  va_list ap;
  va_start(ap, format);
  int n = vsnprintf(buffer, sizeof(buffer), (const char *) format, ap);
  if (n < 0 || ((uint16_t) n >= sizeof(buffer))) {
    protocolPanic("cmd bfr overflow");
    return;
  }
  send_wifi_command(response_parsing_mode, buffer);
  va_end(ap);
}

enum State {
  STATE_IDLE,
  STATE_RESET,

  STATE_SENDING_BATCH_COMMAND,
  STATE_WAITING_BATCH_COMMAND_RESPONSE,

  STATE_UP,
  STATE_WAITING_FOR_STATUS_CMD_RESP,
};

// Current state. Do not set directly, use setState().
static State state;

// Derived from the current state.
static bool is_up;

static PassiveTimer time_in_current_state;

// OK to call for the current state to reset the time in state
// counter.
static void enterState(State new_state) {
  DEBUGF("wifi: %d->%d\n", state, new_state);
  state = new_state;
  time_in_current_state.reset();
  // Derive the is_connected from the new state.
  switch (state) {
    case STATE_UP:
    case STATE_WAITING_FOR_STATUS_CMD_RESP:
      is_up = true;
      break;

    default:
      is_up = false;
      break;
  }
}

enum BatchCommandArgsFormat {
  // No dynamic args.
  FORMAT_ARGS_NONE,
  // Expecting a single dynamic arg: softap SSID.
  FORMAT_ARGS_SSID,
  // Expecting a single dynamic arg: softap password.
  FORMAT_ARGS_PASSWORD,
};

// Common good response codes.
static const char* kResponseSetOk = "Set OK";
static const char* kResponseSuccess = "Success";

struct BatchCommand {
  const char* const format;
  const BatchCommandArgsFormat args;
  const char* okString;
};

static const BatchCommand kBatchCommandList[] = {
    // TODO: use command short cuts to reduce program size.
    { "set system.cmd.mode machine\r\n", FORMAT_ARGS_NONE, kResponseSetOk },
    { "set uart.flow 0 on\r\n", FORMAT_ARGS_NONE, kResponseSetOk },
    { "save\r\n", FORMAT_ARGS_NONE, kResponseSuccess },
    { "reboot\r\n", FORMAT_ARGS_NONE, NULL },

    { "set wlan.antenna.select 1\r\n", FORMAT_ARGS_NONE, kResponseSetOk },
    { "set softap.ssid %s\r\n", FORMAT_ARGS_SSID, kResponseSetOk },
    { "set softap.passkey %s\r\n", FORMAT_ARGS_PASSWORD, kResponseSetOk },
    { "set softap.idle_timeout 3\r\n", FORMAT_ARGS_NONE, kResponseSetOk },

    
    { "set softap.gateway 1.1.1.1\r\n", FORMAT_ARGS_NONE, kResponseSetOk },
    { "set system.cmd.timestamp none\r\n", FORMAT_ARGS_NONE, kResponseSetOk },

    { "network_restart -i softap\r\n", FORMAT_ARGS_NONE, kResponseSuccess },

    { "set gpio.init 16 none\r\n", FORMAT_ARGS_NONE, kResponseSetOk },
};

static const int kBatchCommandCount =
    sizeof(kBatchCommandList) / sizeof(kBatchCommandList[0]);

// Valid only in CONNECTED state. Go through the connection command list.
static int batch_commands_done;

static void sendBatchCommand(int index) {
  DEBUGF("Sending cmd %d\n", index);

  const BatchCommand& batch_cmd = kBatchCommandList[index];

  switch (batch_cmd.args) {
    case FORMAT_ARGS_NONE:
      send_wifi_command(wifi_parser::STD, batch_cmd.format);
      break;

    case FORMAT_ARGS_SSID:
      send_wifi_commandf(wifi_parser::STD, batch_cmd.format, "dashrx");  //ap.ssid);
      break;

    case FORMAT_ARGS_PASSWORD: {
      // The AckMe module requires the special string "\\0" for the no password.
      const char* password = "dashdash";
      send_wifi_commandf(wifi_parser::STD, batch_cmd.format, password);
      break;
  }

  default:
    protocolPanic("unexpected args");
  }
}

// Commands that are sent periodically to the Wifi module.
// Numeric tags should be consecutive since we iterate by incrementing.
// NOTE: for now we have a single status command.
enum StatusCommands {
  // Query the WIFI module for the list of current wifi clients.
  STATUS_CMD_CLIENT_LIST
};

static const StatusCommands kMinStatusCommand = STATUS_CMD_CLIENT_LIST;
static const StatusCommands kMaxStatusCommand = STATUS_CMD_CLIENT_LIST;

static int next_status_cmd = kMinStatusCommand;

// Send next status command. We increment the status command later
// after recieving a response.
static void sendNextStatusCommand() {
  switch (next_status_cmd) {
    case STATUS_CMD_CLIENT_LIST:
      send_wifi_command(wifi_parser::CLIENT_LIST, "get softap.client_list\r\n");
      break;
    default:
      protocolPanic("Unknown status cmd");
  }
}

void polling() {
  wifi_io::polling();
  wifi_parser::polling();

  // Make sure we don't get stucked in the same state for too long.
  if (time_in_current_state.usecs() >= 20 * 1000 * 1000) {
    protocolPanic("State Timeout");
  }

  switch (state) {

    case STATE_IDLE:
      // Assert hardware reset
      wifi_io::wifi_module_control(true);
      // Prevent rx irq buffer overflow until reset stabilizes.
      wifi_io::rx_reset();
      enterState(STATE_RESET);
      break;

    // Here when the wifi module hardware reset pin is active.
    case STATE_RESET: {
      if (time_in_current_state.millis() >= 100) {
        // Release hardware reset after 100ms.
        wifi_io::wifi_module_control(false);
      }

      // NOTE: increase this time to debug initial sequence. It will allow to
      // connect the USB terminal before starting the connection sequence.
      if (time_in_current_state.millis() >= 1000) {
        // Clear rx fifo, just in case.
        wifi_io::rx_reset();
        batch_commands_done = 0;
        enterState(STATE_SENDING_BATCH_COMMAND);
      }
      break;
    }

    // Here when there is at least one more batch command to send.
    case STATE_SENDING_BATCH_COMMAND:
      if (wifi_io::tx_is_empty()) {
        sendBatchCommand(batch_commands_done);
        enterState(STATE_WAITING_BATCH_COMMAND_RESPONSE);
      }
      break;

    case STATE_WAITING_BATCH_COMMAND_RESPONSE: {
      if (!wifi_parser::isResponseParsingDone()) {
        break;
      }
      // This is NULL if for the current batch command we accept any response (free text response).
      const char* okString =
          kBatchCommandList[batch_commands_done].okString;
      if (!okString || (strcmp(okString, wifi_parser::line_buffer) == 0)) {
        DEBUGF("Resp ACCEPTED\n");
        batch_commands_done++;
        if (batch_commands_done >= kBatchCommandCount) {
          DEBUGF("Commands DONE\n");
          enterState(STATE_UP);
          session_id++;
        } else {
          enterState(STATE_SENDING_BATCH_COMMAND);
        }
      } else {
        DEBUGF("Resp IGNORED\n");
      }
      break;
    }

    // Here when the access point is up.
    case STATE_UP:
      // Do nothing if not ready for next status command.
      if (time_in_current_state.millis() < kWifiStatusPollingIntervalMillis || !wifi_io::tx_is_empty()) {
        break;
      }
      // Send next status command
      sendNextStatusCommand();
      enterState(STATE_WAITING_FOR_STATUS_CMD_RESP);
      break;

    case STATE_WAITING_FOR_STATUS_CMD_RESP:
      // Do nothing if response parser is not done.
      if (!wifi_parser::isResponseParsingDone()) {
        break;
      }

      // Increment to next status command
      next_status_cmd = (next_status_cmd >= kMaxStatusCommand)
          ? kMinStatusCommand
          : next_status_cmd + 1;
      if (next_status_cmd != kMinStatusCommand) {
        sendNextStatusCommand();
      } else {
        // Done complete cycle.
        enterState(STATE_UP);
      }
      break;

    default:
      // Will stay forever in this state.
      DEBUGF("Unexpected state: %d\n", state);
      break;
  }
}

void protocolPanic(const char* short_message) {
  // Filtering out if already in state IDLE, in case the panic didn't
  // propagate to all module and will be reisued.
  if (state != STATE_IDLE) {
    PRINTF("** PANIC: %s\n", short_message);
    // Go to the initial state. This will abort the current connection and will
    // try to reconnect.
    enterState(STATE_IDLE);
  } else {
    // TODO: for debugging. Remove after initial stabilization.
    PRINTF("(panic: %s)\n", short_message);
  }
}

void dumpInternalState() {
  PRINTF(
      "WIFI is_up=%d, state=%d, batch_cmds=%d, stat_cmds=%d, ms=%u\n",
      is_up,
      state,
      batch_commands_done,
      next_status_cmd,
      time_in_current_state.millis());
  wifi_parser::dumpInternalState();
  wifi_io::dumpInternalState();
}

void initialize() {
  wifi_io::initialize();
  wifi_parser::initialize();
  next_status_cmd = kMinStatusCommand;
  session_id = 0;
  enterState(STATE_IDLE);
}

}  // namespace wifi
