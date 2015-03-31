// Resources:
// https://github.com/google/protobuf/blob/master/src/google/protobuf/io/coded_stream.cc
// https://github.com/crosswalk-project/chromium-crosswalk/blob/master/google_apis/gcm/protocol/mcs.proto
// https://developers.google.com/protocol-buffers/docs/encoding
// http://en.wikipedia.org/wiki/Protocol_Buffers
// https://github.com/ChromiumWebApps/chromium/tree/master/google_apis/gcm/base

#include "proto_rx.h"

#include "debug.h"
#include "proto_util.h"
#include "esp8266.h"
#include "inttypes.h"

namespace proto_rx {

static uint32_t total_bytes_read = 0;

static bool readByte(uint8_t* b) {
  if (esp8266::rx_fifo.getByte(b)) {
    debug.printf("PR[%02x]\n", *b);
    total_bytes_read++;
    return true;
  }
  return false;
}

//static int bytes_read;
namespace varint_parser {
static uint32_t bytes_parsed;
static uint64_t result;
static bool is_done;

static void reset() {
  bytes_parsed = 0;
  result = 0;
  is_done = false;
}

static bool parse() {
  // NOTE: This is not intended to be called after the first time it
  // returns true but we add this escape just in case.
  if (is_done) {
    return true;
  }

  // Varint64 can be encoded as at most 10 bytes (7 bits per byte)
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
  proto_util::protocolPanic("varint64 overrun");
  return false;
}

}  // namespace varing_parser

class TestListener: Listener {
public:
  virtual void onMessageEnter(uint8_t new_nesting_level, uint8_t tag_num) {
    debug.printf("onMsgEnter: %u, %u\n", new_nesting_level, tag_num);
  }

  virtual void onMessageExit(uint8_t new_nestingLevel) {
    debug.printf("onMsgExit: %u\n", new_nestingLevel);
  }

  virtual void onVarintField(uint8_t tag_num, uint64_t value) {
    debug.printf("onVarint: %u, %08x %08x\n", tag_num,
        static_cast<uint32_t>(value >> 32), static_cast<uint32_t>(value));
  }

  virtual bool isSubMessage(uint8_t tag_num, uint32_t length) {
    debug.printf("resolve: %u, %u\n", tag_num, length);
    return false;
  }

  // Called for variable length fields that were resolved as VAR_LEN_DATA.
  virtual void onDataFieldStart(uint8_t tag_num) {
    debug.printf("onDataStart: %u\n", tag_num);
  }
  virtual void onDataFieldByte(uint8_t tag_num, uint8_t byte_value) {
    debug.printf("onDataByte: %u\n", tag_num);
  }
  virtual void onDataFieldEnd(uint8_t tag_num) {
    debug.printf("onDataEnd: %u\n", tag_num);
  }
};

static TestListener test_listener;
static Listener* listener;

enum State {
  STATE_STOPED,  // 0
  STATE_PARSE_VERSION,  // 1
  STATE_PARSE_TOP_MSG_TAG,  // 2
  STATE_PARSE_TOP_MSG_LENGTH,  // 3
  STATE_TEST_IF_MSG_DONE,  // 4
  STATE_PARSE_FIELD_TAG,  // 5
  STATE_PARSE_VARINT_FIELD_VALUE,  // 6
//  STATE_PARSE_FIXED32_VALUE,  // 7
//  STATE_PARSE_FIXED64_VALUE,  // 8
  STATE_PARSE_FIELD_DATA_LENGTH,  // 9
  STATE_PARSE_VAR_LEN_DATA,  // 10
  //STATE_EVENT_READY,
};

static State state;

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

static char stack_path[20];

// Encode the stack pass as a null terminated string with a '.' seperated list of message_tag_num
// in decimal format, starting with top level message. E.g. "3.2.11".
static void updateStackPath() {
  int i = 0;
  stack_path[0] = '\0';
  const int kBufferSize = sizeof(stack_path);
  for(int j = 0; j < stack_size; j++) {
    const uint8_t tag_num = stack[j].message_tag_num;
    i += snprintf(&stack_path[i], (kBufferSize - i), j? ".%u" : "%u", tag_num);
    // Check for buffer overflow.
    if (i >= kBufferSize - 1) {
      proto_util::protocolPanic("path ovf");
      return;
    }
  }
  debug.printf("PATH: [%s]\n", stack_path);
}

static uint8_t field_tag_num;
static uint8_t field_tag_type;

static uint32_t field_var_length;
static uint32_t field_var_bytes_read;

void setup() {
  state = STATE_STOPED;
  varint_parser::reset();
}

static void setState(State new_state) {
  debug.printf("parser %d -> %d\n", state, new_state);
  state = new_state;
  // NOTE: some state changes don't require varint parser reset but we do
  // it anyway for simplicity.
  varint_parser::reset();
}

void start() {
  setState(STATE_PARSE_VERSION);
  stack_size = 0;
  updateStackPath();
}

void stop() {
  setState(STATE_STOPED);
}

void loop() {
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
        // We use uintt_8 to represent tag nums.
        if (varint_parser::result > 0xff) {
          proto_util::protocolPanic("tag size");
          state = STATE_STOPED;
          return;
        }
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
        setState(STATE_TEST_IF_MSG_DONE);
      }
      break;

    case STATE_TEST_IF_MSG_DONE: {
      const StackEntry* const stack_top = &stack[stack_size - 1];
      const uint32_t bytes_so_far = total_bytes_read
          - stack_top->start_total_bytes_read;
      debug.printf("\nMsg bytes: %u/%u (level=%d)\n", bytes_so_far, stack_top->message_length, stack_size);
      //  TODO: if it's actually > than protcol panic. Should match exactly.
      if (bytes_so_far >= stack_top->message_length) {
        stack_size--;
        debug.printf("\n*** POP -> %d\n", stack_size);
        // In pop the path is shorten so we don't worry about the length.
        updateStackPath();
        if (stack_size == 0) {
          // Done parsing top level message.
          setState(STATE_PARSE_TOP_MSG_TAG);
        }
      } else {
        // Continue pasring fields of this message.
        setState(STATE_PARSE_FIELD_TAG);
      }
    }
      break;

    case STATE_PARSE_FIELD_TAG:
      if (varint_parser::parse()) {
        // TODO: panic if tag num doesn't fit in field_tag_num.
        field_tag_num = static_cast<uint8_t>(varint_parser::result >> 3);
        field_tag_type = static_cast<uint8_t>(varint_parser::result) & 0x7;
        debug.printf("TAG: [%s].%u, type=%d\n", stack_path, field_tag_num, field_tag_type);
        // Dispatch by field tag type
        if (field_tag_type == proto_util::kTagTypeVarint) {
          setState(STATE_PARSE_VARINT_FIELD_VALUE);
        } else if (field_tag_type == proto_util::kTagTypeLenDelimited) {
          setState(STATE_PARSE_FIELD_DATA_LENGTH);
        } else {
          proto_util::protocolPanic("parser tag type");
          setState(STATE_STOPED);
        }
      }
      break;

    case STATE_PARSE_VARINT_FIELD_VALUE:
      if (varint_parser::parse()) {
        debug.printf("VARINT FIELD DONE [%s].%u\n", stack_path, field_tag_num);
        setState(STATE_TEST_IF_MSG_DONE);
      }
      break;


    case STATE_PARSE_FIELD_DATA_LENGTH:
      if (varint_parser::parse()) {
        field_var_length = static_cast<uint32_t>(varint_parser::result);

        // TODO: ask listener for policy. For now faking it.

        bool is_sub_message = false;
//        if (stack_size == 1 &&
//            stack[0].message_tag_num == 3 &&  // LoginResponse
//            field_tag_num == 3) {             // LoginResponse.ErrorInfo
//          debug.printf("\nEntering LoginResponse.ErrorInfo sub message [%s].%u\n", stack_path, field_tag_num);
//          is_sub_message = true;
//        }
//        if (stack_size == 2 &&
//            stack[0].message_tag_num == 3 &&  // LoginResponse
//            stack[1].message_tag_num == 3 &&  // LoginResponse.ErrorInfo
//            field_tag_num == 4) {             // LoginResponse.ErrorInfo.Extension
//          debug.printf("\nEntering LoginResponse.ErrorInfo.Extension sub message [%s].%u\n", stack_path, field_tag_num);
//          is_sub_message = true;
//        }

        if (strcmp("3", stack_path) == 0 && field_tag_num == 3) {
            is_sub_message = true;
            debug.printf("Entering LoginResponse.ErrorInfo\n");
        } else  if (strcmp("3.3", stack_path) == 0 && field_tag_num == 4) {
          is_sub_message = true;
          debug.printf("Entering LoginResponse.ErrorInfo.Extension sub message\n");
        }


        if (is_sub_message) {
          stack_size++;
          if (stack_size > kMaxStackSize) {
            // TODO: make this a common method to set panic and stop
            proto_util::protocolPanic("parser stack");
            setState(STATE_STOPED);
            return;
          }
          debug.printf("\n***PUSH -> %d\n", stack_size);
          StackEntry* const stack_top = &stack[stack_size-1];
          stack_top->message_length = field_var_length;
          stack_top->message_tag_num = field_tag_num;
          stack_top->start_total_bytes_read = total_bytes_read;
          // Raised panic if buffer overflow.
          updateStackPath();
          setState(STATE_TEST_IF_MSG_DONE);
        } else {
          setState(STATE_PARSE_VAR_LEN_DATA);
          field_var_bytes_read = 0;
        }

        //FieldPolicy policy =  VAR_LEN_SKIP;
        //setState(STATE_PARSE_VAR_LEN_DATA);
        //field_var_bytes_read = 0;
      }
      break;

    case STATE_PARSE_VAR_LEN_DATA:
      while (field_var_bytes_read < field_var_length) {
        uint8_t b;
        if (!readByte(&b)) {
          return;
        }

        debug.printf("data %u/%u: ", field_var_bytes_read, field_var_length);
        // TODO: make this a common util function.
        if (b == '\r') {
          debug.puts("{CR}");
        } else if (b == '\n') {
          debug.puts("{LF}");
        } else if (b > ' ' && b <= '~') {
          debug.printf("{%c}", b);
        } else {
          debug.printf("{%02x}", b);
        }
        debug.puts("\n");

        field_var_bytes_read++;
      }
      debug.printf("VAR LEN FIELD DONE [%s].%u, len=%u\n", stack_path, field_tag_num, field_var_bytes_read);
      setState(STATE_TEST_IF_MSG_DONE);
      break;

    default:
      proto_util::protocolPanic("parser state");
  }
}


//static const char kDomain[] = "mcs.android.com";
//
//static char tmp_buffer[30];
//
//
//static char bfr[20+1];
//
//static char* uint64ToDecimal(uint64_t v) {
//  char* p = bfr + sizeof(bfr);
//  *(--p) = '\0';
//  for (bool first = true; v || first; first = false) {
//    uint32_t digit = v % 10;
//    const char c = '0' + digit;
//    *(--p) = c;
//    v = v / 10;
//  }
//  return p;
//}
//
//static char* uint64ToHex(uint64_t v) {
//  char* p = bfr + sizeof(bfr);
//  *(--p) = '\0';
//  for (bool first = true; v || first; first = false) {
//    uint32_t digit = v & 0xf;
//    const char c = (digit < 10)
//        ? ('0' + digit)
//        : ('a' - 10 + digit);
//    *(--p) = c;
//    v = v >> 4;
//  }
//  return p;
//}

// ---- TX

// Sending of messages is done in two passes to keep the memory footprint small. The first
// is a dry run pass that just counts the number of bytes sent and the second is the actual
// message sending which is prefixed by the message size.

//void sendProtocolVersion() {
//  protobuf::setCountingOnlyMode(false);
//  protobuf::writeVarint(38);
//}

//void sendLoginRequestAndFlush(uint64_t device_id, uint64_t auth_token) {
//  debug.printf("id=%08x:%08x\n", static_cast<uint32_t>(device_id >> 32),
//      static_cast<uint32_t>(device_id));
//  debug.printf("auth=%08x:%08x\n", static_cast<uint32_t>(auth_token >> 32),
//      static_cast<uint32_t>(auth_token));
//  uint32_t pass0_size = 0;
//  uint32_t pass1_size = 0;
//
//  protobuf::setCountingOnlyMode(true);
//
//  for (int pass = 0; pass < 2; pass++) {
//    const uint32_t pass_start_count = protobuf::totalBytesWritten();
//
//    protobuf::writeStringField(1, "");  // Not used. Required field.
//    protobuf::writeStringField(2, kDomain);  // domain
//    sniprintf(tmp_buffer, sizeof(tmp_buffer), "%llu", device_id);
//    protobuf::writeStringField(3, tmp_buffer);  // user
//    protobuf::writeStringField(4, tmp_buffer);  // resource
//    sniprintf(tmp_buffer, sizeof(tmp_buffer), "%llu", auth_token);
//    protobuf::writeStringField(5, tmp_buffer);  // auth token
//    sniprintf(tmp_buffer, sizeof(tmp_buffer), "android-%llx", device_id);
//    protobuf::writeStringField(6, tmp_buffer);  // device_id
//    protobuf::writeVarintField(16, 2);  // auth_service = ANDROID_ID
//
//    if (pass == 0) {
//      pass0_size = protobuf::totalBytesWritten() - pass_start_count;
//      protobuf::setCountingOnlyMode(false);
//      // Send message header
//      protobuf::writeVarint(2);  // message tag for LoginRequest.
//      protobuf::writeVarint(pass0_size);
//    } else {
//      // This else section is for verification only.
//      pass1_size = protobuf::totalBytesWritten() - pass_start_count;
//    }
//  }
//
//  protobuf::flushWrites();
//  debug.printf("LoginRequest sent, %u (%u) bytes\n", pass1_size, pass0_size);
//}


//void sendProtocolVersion() {
//  esp8266::rx_fifo.putByte(38);
//  //esp8266::rx_fifo.putByte(1);
//}

//void sendLoginRequest(uint64_t device_id, uint64_t auth_token) {
//  debug.printf("id=%08x:%08x\n", static_cast<uint32_t>(device_id >> 32),
//      static_cast<uint32_t>(device_id));
//  debug.printf("auth=%08x:%08x\n", static_cast<uint32_t>(auth_token >> 32),
//      static_cast<uint32_t>(auth_token));
//
////
////  debug.printf("x0=%x\n", auth_token);
////  debug.printf("x1=%lx\n", auth_token);
////  debug.printf("x2=%llx\n", auth_token);
////
////  debug.printf("u0=%u\n", auth_token);
////  debug.printf("u1=%lu\n", auth_token);
////  debug.printf("u2=%llu\n", auth_token);
//
//  uint32_t pass0_size = 0;
//  uint32_t pass1_size = 0;
//
//  protobuf::setCountingOnlyMode(true);
//
//  for (int pass = 0; pass < 2; pass++) {
//    const uint32_t pass_start_count = protobuf::totalBytesWritten();
//
//    protobuf::writeStringField(1, "");  // Not used. Required field.
//    protobuf::writeStringField(2, kDomain);  // domain
//
//    const char* temp_num = uint64ToDecimal(device_id);
//    //snprintf(tmp_buffer, sizeof(tmp_buffer), "%llu", device_id);
//    protobuf::writeStringField(3, temp_num);  // user
//    protobuf::writeStringField(4, temp_num);  // resource
//
//    temp_num = uint64ToDecimal(auth_token);
//    //snprintf(tmp_buffer, sizeof(tmp_buffer), "%llu", auth_token);
//    protobuf::writeStringField(5, temp_num);  // auth token
//
//    temp_num = uint64ToHex(device_id);
//    snprintf(tmp_buffer, sizeof(tmp_buffer), "android-%s", temp_num);
//    protobuf::writeStringField(6, tmp_buffer);  // device_id
//
//    protobuf::writeVarintField(16, 2);  // auth_service = ANDROID_ID
//
//    if (pass == 0) {
//      pass0_size = protobuf::totalBytesWritten() - pass_start_count;
//      protobuf::setCountingOnlyMode(false);
//      // Send message header
//      protobuf::writeVarint(2);  // message tag for LoginRequest.
//      protobuf::writeVarint(pass0_size);
//    } else {
//      // This else section is for verification only.
//      pass1_size = protobuf::totalBytesWritten() - pass_start_count;
//    }
//  }
//
//  //protobuf::flushWrites();
//  debug.printf("LoginRequest sent, %u (%u) bytes\n", pass1_size, pass0_size);
//}

// ---- RX

//enum ParserId {
//  PARSER_VERSION_NUMBER,
//  PARSER_MESSAGE_FRAME,
//  PARSER_LOGIN_RESPONSE,
//};
//
//// ---------- Protocol Version
//
//namespace protocol_version_parser {
//static bool parse() {
//  uint32_t remote_version;
//  return protobuf::readVarint32(&remote_version);
//}
//}  // namespace protocol_version_parser
//
//// ---------- Message Frame Header Parser
//
//namespace message_frame_parser {
//static uint32_t message_tag;
//static uint32_t message_size;
//
//static bool has_message_tag;
//
//static void reset() {
//  has_message_tag = false;
//}
//
//static bool parse() {
//  //debug.printf("?frame\n");
//  if (!has_message_tag) {
//    has_message_tag = protobuf::readVarint32(&message_tag);
//  }
//  return has_message_tag && protobuf::readVarint32(&message_size);
//}
//}  // message_frame_parser
//
//// ---------- LoginResponse Parser
//
//
//
//
//
//namespace login_response_parser {
//enum State {
//  PARSING_FIELD_TAG,
//  PARSING_FIELD_VARINT_VALUE,
//  PARSING_FIELD_VAR_LEN_SIZE,
//  PARSING_FIELD_VAR_LEN_DATA,
//};
//
//static State state;
//static uint32_t field_tag_num;
//static uint8_t field_tag_type;
//
//static uint64_t varint_value;
//
//static uint32_t var_len_length;
//static uint32_t var_len_read;
//
//// TODO: expose this count from protobuf.h, in case it may use internal
//// buffering.
//static uint32_t start_input_marker;
//
//static void start() {
//  state = PARSING_FIELD_TAG;
//  start_input_marker = esp8266::rx_fifo.bytesConsumed();
//}
//
//static uint32_t bytesRead() {
//  // Should give the correct answer even if a wrap around occured.
//  return esp8266::rx_fifo.bytesConsumed() - start_input_marker;
//}
//
//static bool parse() {
//  // TODO: if parsed the entire message (by size) then return true.
//
//  if (state == PARSING_FIELD_TAG) {
//    if (bytesRead() >= message_frame_parser::message_size) {
//      debug.printf("Login: read %d/%d bytes\n", bytesRead(), message_frame_parser::message_size);
//      return true;
//    }
//    if (protobuf::readFieldTag(&field_tag_num, &field_tag_type)) {
//      debug.printf("\n---tag_num: %d, tag_type=%d\n", field_tag_num, field_tag_type);
//      switch (field_tag_type) {
//        case protobuf::kTagTypeVarint:
//          state = PARSING_FIELD_VARINT_VALUE;
//          debug.printf("login > VARIANT_VALUE\n");
//          break;
//        case protobuf::kTagTypeLenDelimited:
//          debug.printf("login > VAR_SIZE\n");
//          state = PARSING_FIELD_VAR_LEN_SIZE;
//          break;
//        default:
//          protobuf::protocolPanic("field type");
//      }
//    }
//
//  } else if (state == PARSING_FIELD_VARINT_VALUE) {
//    if (protobuf::readVarint64(&varint_value)) {
//      debug.printf("\nlogin > TAG\n");
//      state = PARSING_FIELD_TAG;
//    }
//
//  } else if (state == PARSING_FIELD_VAR_LEN_SIZE) {
//    if (protobuf::readVarint32(&var_len_length)) {
//      var_len_read = 0;
//      debug.printf("login > VAR_DATA\n");
//      state = PARSING_FIELD_VAR_LEN_DATA;
//    }
//  }
//
//  else if (state == PARSING_FIELD_VAR_LEN_DATA) {
//    while (var_len_read < var_len_length) {
//       uint8_t b;
//      if (!protobuf::readRawByte(&b)) {
//        return false;
//      }
//      debug.printf("var data: %u/%u\n", var_len_read, var_len_length);
//      var_len_read++;
//    }
//    debug.printf("\nlogin > TAG\n");
//    state = PARSING_FIELD_TAG;
//  } else {
//    protobuf::protocolPanic("login resp state");
//  }
//  return false;
//}
//
//}  // login_response_parser
//
//// ----------
//
//static ParserId current_parser_id;
//static EventType current_event;
////static bool current_parser_is_done;
//
//extern EventType currentEvent() {
//  return current_event;
////  if (!current_parser_is_done) {
////    return EVENT_NONE;
////  }
////  switch (current_parser_id) {
////    case PARSER_VERSION_NUMBER:
////      // Event is ready.
////      return EVENT_PROTOCOL_VERSION;
////    case PARSER_MESSAGE_FRAME:
////      return EVENT_NONE;
////    case PARSER_LOGIN_RESPONSE:
////      // Event is ready
////      return EVENT_LOGIN_RESPONSE;
////    default:
////      protobuf::protocolPanic("unknown parser");
////      return EVENT_LOGIN_RESPONSE;
////  }
//}
//
//extern void nextEvent() {
//  if (currentEvent()) {
//    debug.printf("next event\n");
//    current_parser_id = PARSER_MESSAGE_FRAME;
//    current_event = EVENT_NONE;
//    message_frame_parser::reset();
//  } else {
//    debug.printf("next event SKIPPED\n");
//  }
//}
//
//static void loop_rx() {
//  // Do nothing if not connected there is an event waiting for consumption.
//  if (!esp8266::connectionId() || current_event) {
//    return;
//  }
//
//  switch (current_parser_id) {
//    case PARSER_VERSION_NUMBER:
//      if (protocol_version_parser::parse()) {
//        debug.printf("version parser done\n");
//        current_parser_id = PARSER_MESSAGE_FRAME;
//        message_frame_parser::reset();
//        debug.printf("installed frame parser\n");
//      }
//      return;
//
//    case PARSER_MESSAGE_FRAME:
//      if (!message_frame_parser::parse()) {
//        return;
//      }
//      //current_parser_is_done = true;
//      debug.printf("frame parser done (%u, %u)\n",
//          message_frame_parser::message_tag,
//          message_frame_parser::message_size);
//
//      // We parsed the message header but now need to parse also the message
//      // to have the complete event. Dispatch to message parser by message tag
//      // from the frame.
//      if (message_frame_parser::message_tag == 3) {
//        current_parser_id = PARSER_LOGIN_RESPONSE;
//        login_response_parser::start();
//        debug.printf("installed login parser\n");
//
//        return;
//      }
//      protobuf::protocolPanic("Unknown frame tag");
//      return;
//
//    case PARSER_LOGIN_RESPONSE:
//      if (!login_response_parser::parse()) {
//        return;
//      }
//      current_event = EVENT_LOGIN_RESPONSE;
//      debug.printf("login parser done\n");
//      // Login response event is ready for consumption.
//      return;
//
//    default:
//      protobuf::protocolPanic("Unknown parser");
//      return;
//  }
//}

// ---- MISC

//void loop() {
//  //protobuf::loop();
//  loop_rx();
//}

//void setup() {
//  //protobuf::setup();
//}

//void reset() {
//  //protobuf::reset();
////  current_parser_id = PARSER_VERSION_NUMBER;
////  current_event = EVENT_NONE;
//}

//void dumpState() {
//  debug.printf(">>> parser=%d, event=%d, login_bytes=%u\n", current_parser_id, current_event, login_response_parser::bytesRead());
//}
}  // namespace proto_rx
