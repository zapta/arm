#include "protocol_rx.h"

#include "debug.h"
#include "protocol_util.h"
#include "esp8266.h"
#include "string_util.h"
#include "string_pool.h"

namespace protocol_rx {

enum State {
  STATE_STOPED,  // 0
  STATE_PARSE_VERSION,  // 1
  STATE_PARSE_TOP_MSG_TAG,  // 2
  STATE_PARSE_TOP_MSG_LENGTH,  // 3
  STATE_TEST_IF_MSG_DONE,  // 4
  STATE_PARSE_FIELD_TAG,  // 5
  STATE_PARSE_VARINT_FIELD_VALUE,  // 6
  STATE_PARSE_FIELD_DATA_LENGTH,  // 7
  STATE_PARSE_VAR_LEN_DATA,  // 8
  STATE_EVENT_READY,  // 9
};

static State state;
static uint32_t total_bytes_read;

static EventType pending_event_type;

static char event_string_pool_buffer[50];
static StringPool event_string_pool(event_string_pool_buffer, sizeof(event_string_pool_buffer));

static bool readByte(uint8_t* b) {
  if (esp8266::rx_fifo.getByte(b)) {
    total_bytes_read++;
    return true;
  }
  return false;
}

// Parser for an incoming protocol buffers variable length int.
// The value is available at 'result' once is_done is set.
namespace varint_parser {
static uint32_t bytes_parsed;
static uint64_t result;
static bool is_done;

static void reset() {
  bytes_parsed = 0;
  result = 0;
  is_done = false;
}

// Keep calling this until it returns true. At that point, result is at
// 'result'.
static bool parse() {
  if (is_done) {
    return true;
  }

  // A varint64 can be encoded as at most 10 bytes (7 bits per byte)
  while (bytes_parsed < 10) {
    uint8_t b;
    if (!readByte(&b)) {
      return false;
    }
    result |= static_cast<uint64_t>(b & 0x7F) << (7 * bytes_parsed);
    bytes_parsed++;
    if (!(b & 0x80)) {
      is_done = true;
      debug.printf("varint: %08x %08x\n", static_cast<uint32_t>(result >> 32),
          static_cast<uint32_t>(result));
      return true;
    }
  }
  protocol::protocolPanic("varint64 overrun");
  return false;
}
}  // namespace varint_parser

// Represents one messages in the message stack.
struct StackEntry {
  // For top level message (stack_size == 1), this the the message tag. For sub messages
  // (stack_size > 1), this is the field tag in the enclosing message.
  uint8_t message_tag_num;
  // Total message size. Used to determine the message end.
  uint32_t message_length;
  // The value of total_bytes_read before reading the first field. Used to
  // determine the end of the message.
  uint32_t start_total_bytes_read;
};

static const int kMaxStackSize = 4;
static StackEntry stack[kMaxStackSize];
// Stack size. 0 is idle, 1 is the top message, 2 is its sub message and so on.
// This is also the number of items in the stack.
static int stack_size;

// A string that encodes the unique tag path of the top message in
// the stack. See updateStackPath for the format.
static char stack_path[20];

// Convinience function to test the current message path.
static inline bool stackPathEq(const char* s) {
  return strcmp(stack_path, s) == 0;
}

// Encode the stack pass as a null terminated string with a '.' seperated list of message_tag_num
// in decimal format, starting with top level message. E.g. "3.2.11".
static void updateStackPath() {
  int i = 0;
  stack_path[0] = '\0';
  const int kBufferSize = sizeof(stack_path);
  for (int j = 0; j < stack_size; j++) {
    const uint8_t tag_num = stack[j].message_tag_num;
    i += snprintf(&stack_path[i], (kBufferSize - i), j ? ".%u" : "%u", tag_num);
    // Check for buffer overflow.
    if (i >= kBufferSize - 1) {
      protocol::protocolPanic("path ovf");
      return;
    }
  }
  debug.printf("PATH: [%s]\n", stack_path);
}
// The tag number and type of the current field.
static uint8_t field_tag_num;
static uint8_t field_tag_type;

// The total size of the current variable length field and the number of
// bytes read so far.
static uint32_t field_var_length;
static uint32_t field_var_bytes_read;

// The parsing of each top level message (and it sub messages) is done
// against a listener on the parsing events. This is the base
// listener type and is also the default one for to level message
// we want to ignore.
class ProtoListener {
public:
  // Call backs are done as long as the protocol parsing is on track.
  virtual const char* listenerName() {
    return "DEFAULT";
  }
  // This method also resets the listener.
  virtual void onTopMessageStart() {
  }
  virtual void onTopMessageEnd() {
  }
  virtual void onSubMessageStart() {
  }
  virtual void onSubMessageEnd() {
  }
  virtual void onVarintField() {
  }
  // After parsing the tag and length of a variable length field,
  // this method is called to resolve if the field is a data field
  // (e.g. a string) or a sub message. This information cannot be derived
  // from the primitive protocol buffer elements.
  virtual bool isCurrentFieldASubMessage() {
    return false;
  }
  virtual void onDataFieldStart() {
  }
  virtual void onDataFieldByte(uint8_t byte_value) {
  }
  virtual void onDataFieldEnd() {
  }
};
static ProtoListener default_listener;

// ----- LoginResponse
class LoginResponseListener: public ProtoListener {
public:
  virtual const char* listenerName() {
    return "LOGIN_RESP";
  }
  virtual bool isCurrentFieldASubMessage() {
    // Error info, ErrorInfo.Extension.
    return (stackPathEq("3")
        && (field_tag_num == 3 || field_tag_num == 4 || field_tag_num == 7))
        || (stackPathEq("3.3") && field_tag_num == 4);
  }
  virtual void onTopMessageStart() {
    rx_login_response_event.error_code = 0;
  }
  virtual void onTopMessageEnd() {
    pending_event_type = EVENT_LOGIN_RESPONSE;
  }
  virtual void onVarintField() {
    if (stackPathEq("3.3") && field_tag_num == 1) {
      rx_login_response_event.error_code =
          static_cast<int32_t>(varint_parser::result);
    }
  }
};
static LoginResponseListener login_response_listener;
RxLoginResponseEvent rx_login_response_event;

// ----- DataMessageStanza (8)

class DataMessageStanzaListener: public ProtoListener {
public:
  virtual const char* listenerName() {
    return "DM_STANZA";
  }
  virtual bool isCurrentFieldASubMessage() {
    // App data
    return (stackPathEq("8") && field_tag_num == 7);
  }
  virtual void onTopMessageStart() {
    _app_data_field_count = 0;
    rx_data_message_stanza_event.value = "";
  }
  virtual void onTopMessageEnd() {
    pending_event_type = EVENT_DATA_MESSAGE_STANZA;
  }
  virtual void onSubMessageStart() {
    if (stackPathEq("8.7")) {
      _app_data_field_count++;
    }
  }
  void onDataFieldStart() {
     if (isTargetValueField()) {
       // Create an extensible string.
       rx_data_message_stanza_event.value = event_string_pool.createNewString();
     }
   }
  void onDataFieldByte(uint8_t byte_value) {
    if (isTargetValueField()) {
      event_string_pool.addByteToLastString(byte_value);
    }
  }

private:
  // We extract the text from the value field of the first AppData sub message.
  bool isTargetValueField() {
    return stackPathEq("8.7") && _app_data_field_count == 1 && field_tag_num == 2;
  }

  // Counts repeated AppData field. First is 1, second is 2, etc.
  int _app_data_field_count;
};
static DataMessageStanzaListener data_message_stanza_listener;
RxDataMessageStanzaEvent rx_data_message_stanza_event;

// -----

// Never null. Changes listener on incoming message boundary.
static ProtoListener* current_listener = &default_listener;

void initialize() {
  state = STATE_STOPED;
  varint_parser::reset();
}

static void setState(State new_state) {
  debug.printf("parser %d -> %d\n", state, new_state);
  state = new_state;
  varint_parser::reset();
}

void resetForANewConnection() {
  setState(STATE_PARSE_VERSION);
  stack_size = 0;
  updateStackPath();
}

void polling() {
  switch (state) {
    case STATE_STOPED:
      break;

    case STATE_PARSE_VERSION:
      if (varint_parser::parse()) {
        setState(STATE_PARSE_TOP_MSG_TAG);
      }
      break;

    case STATE_PARSE_TOP_MSG_TAG:
      // TODO: assert that stack_size == 0 here.
      if (varint_parser::parse()) {
        // We use uint_8 to represent tag nums. 8 bits are sufficient for this
        // specific protocol.
        if (varint_parser::result > 0xff) {
          protocol::protocolPanic("tag size");
          setState(STATE_STOPED);
          return;
        }
        pending_event_type = EVENT_NONE;
        stack[0].message_tag_num = static_cast<uint8_t>(varint_parser::result);
        setState(STATE_PARSE_TOP_MSG_LENGTH);
      }
      break;

    case STATE_PARSE_TOP_MSG_LENGTH:
      // TODO: assert that stack_size is 0 here.
      if (varint_parser::parse()) {
        // NOTE: message_tag_num was written in the previous state STATE_PARSE_TOP_MSG_TAG.
        stack[0].message_length = static_cast<uint32_t>(varint_parser::result);
        stack[0].start_total_bytes_read = total_bytes_read;
        // This pushes the top level message (first in the stack).
        stack_size = 1;
        updateStackPath();
        debug.printf("\n***PUSH -> %d\n", stack_size);
        debug.printf("*** msg tag num: %d\n", stack[0].message_tag_num);
        protocol_util::in_messages_counter++;
        switch (stack[0].message_tag_num) {
          case 3:
            current_listener = &login_response_listener;
            break;
          case 8:
            current_listener = &data_message_stanza_listener;
            break;
          default:
            current_listener = &default_listener;
            debug.printf("LISTENER: %s\n", current_listener->listenerName());
        }
        event_string_pool.reset();
        current_listener->onTopMessageStart();
        setState(STATE_TEST_IF_MSG_DONE);
      }
      break;

    case STATE_TEST_IF_MSG_DONE: {
      const StackEntry* const stack_top = &stack[stack_size - 1];
      const uint32_t bytes_so_far = total_bytes_read
          - stack_top->start_total_bytes_read;
      debug.printf("\nMsg bytes read: %u/%u (level=%d)\n", bytes_so_far,
          stack_top->message_length, stack_size);
      //  TODO: if it's actually > than protcol panic. Should match exactly.
      if (bytes_so_far >= stack_top->message_length) {
        if (stack_size == 1) {
          current_listener->onTopMessageEnd();
        } else {
          current_listener->onSubMessageEnd();
        }
        stack_size--;
        debug.printf("\n*** POP -> %d\n", stack_size);
        // In pop the path is shorten so we don't worry about the length.
        updateStackPath();
        if (stack_size == 0) {
          // Done parsing a top level message. If the message generated an
          // event, go to STATE_EVENT_READY state and stay there until the event
          // will be processed by the app. Otherwise, continue parsing next
          // top level message.
          setState(
              pending_event_type ? STATE_EVENT_READY : STATE_PARSE_TOP_MSG_TAG);
        }
      } else {
        // More bytes left in this message. Parse next field in this message.
        setState(STATE_PARSE_FIELD_TAG);
      }
      break;
    }

    case STATE_PARSE_FIELD_TAG:
      if (varint_parser::parse()) {
        // TODO: panic if tag num doesn't fit in field_tag_num.
        field_tag_num = static_cast<uint8_t>(varint_parser::result >> 3);
        field_tag_type = static_cast<uint8_t>(varint_parser::result) & 0x7;
        debug.printf("TAG: [%s].%u, type=%d\n", stack_path, field_tag_num,
            field_tag_type);
        // Dispatch by field tag type
        if (field_tag_type == protocol_util::kTagTypeVarint) {
          setState(STATE_PARSE_VARINT_FIELD_VALUE);
        } else if (field_tag_type == protocol_util::kTagTypeLenDelimited) {
          setState(STATE_PARSE_FIELD_DATA_LENGTH);
        } else {
          protocol::protocolPanic("parser tag type");
          setState(STATE_STOPED);
        }
      }
      break;

    case STATE_PARSE_VARINT_FIELD_VALUE:
      if (varint_parser::parse()) {
        debug.printf("VARINT FIELD DONE [%s].%u\n", stack_path, field_tag_num);
        current_listener->onVarintField();
        setState(STATE_TEST_IF_MSG_DONE);
      }
      break;

    case STATE_PARSE_FIELD_DATA_LENGTH:
      if (varint_parser::parse()) {
        field_var_length = static_cast<uint32_t>(varint_parser::result);

        // This decision is message dependent so we ask the listener.
        if (current_listener->isCurrentFieldASubMessage()) {
          stack_size++;
          if (stack_size > kMaxStackSize) {
            // TODO: make this a common method to set panic and stop
            protocol::protocolPanic("parser stack");
            setState(STATE_STOPED);
            return;
          }
          debug.printf("\n***PUSH -> %d\n", stack_size);
          StackEntry* const stack_top = &stack[stack_size - 1];
          stack_top->message_length = field_var_length;
          stack_top->message_tag_num = field_tag_num;
          stack_top->start_total_bytes_read = total_bytes_read;
          updateStackPath();
          current_listener->onSubMessageStart();
          setState(STATE_TEST_IF_MSG_DONE);
        } else {
          current_listener->onDataFieldStart();
          setState(STATE_PARSE_VAR_LEN_DATA);
          field_var_bytes_read = 0;
        }
      }
      break;

    case STATE_PARSE_VAR_LEN_DATA:
      while (field_var_bytes_read < field_var_length) {
        uint8_t b;
        if (!readByte(&b)) {
          return;
        }

        // For debugging only. 
        // Use with caution, it slows the incoming stream procesing and creates timing issues.
//        debug.printf("data %u/%u: ", field_var_bytes_read, field_var_length);
//        // TODO: make this a common util function.
//        if (b == '\r') {
//          debug.puts("{CR}");
//        } else if (b == '\n') {
//          debug.puts("{LF}");
//        } else if (b > ' ' && b <= '~') {
//          debug.printf("{%c}", b);
//        } else {
//          debug.printf("{%02x}", b);
//        }
//        debug.puts("\n");

        field_var_bytes_read++;
        current_listener->onDataFieldByte(b);
      }
      debug.printf("VAR LEN FIELD DONE [%s].%u, len=%u\n", stack_path,
          field_tag_num, field_var_bytes_read);
      current_listener->onDataFieldEnd();
      setState(STATE_TEST_IF_MSG_DONE);
      break;

    case STATE_EVENT_READY:
      // Stay in this state until eventDone() is called
      break;

    default:
      debug.printf("state=%d\n", state);
      protocol::protocolPanic("parser state");
  }
}

EventType currentEvent() {
  return (state == STATE_EVENT_READY) ? pending_event_type : EVENT_NONE;
}
extern void eventDone() {
  debug.printf("eventDone() called\n");
  if (state == STATE_EVENT_READY) {
    setState(STATE_PARSE_TOP_MSG_TAG);
  }
}

// Do not call directly from this module. Call protocol::protocolPanic() instead
// so the panic mode can be propograted to all stake holders.
void onProtocolPanic() {
  // TODO: do something useful or delete this TODO comment.
}

void dumpInternalState() {
  debug.printf("proto_rx: depth=%d, path=[%s], state=%d, event=%d\n", stack_size,  stack_path, state, currentEvent());
}

}  // namespace protocol_rx
