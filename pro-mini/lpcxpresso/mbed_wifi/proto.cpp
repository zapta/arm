// Resources:
// https://github.com/google/protobuf/blob/master/src/google/protobuf/io/coded_stream.cc
// https://github.com/crosswalk-project/chromium-crosswalk/blob/master/google_apis/gcm/protocol/mcs.proto
// https://developers.google.com/protocol-buffers/docs/encoding
// http://en.wikipedia.org/wiki/Protocol_Buffers
// https://github.com/ChromiumWebApps/chromium/tree/master/google_apis/gcm/base

#include "proto.h"

#include "debug.h"
#include "proto_util.h"
#include "proto_tx.h"
#include "proto_rx.h"
#include "esp8266.h"
#include "inttypes.h"

namespace proto {

//static const char kDomain[] = "mcs.android.com";
//
//static char tmp_buffer[30];


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

void setup() {
  proto_tx::setup();
  proto_rx::setup();
  proto_util::is_panic_mode = false;

  //protobuf::setup();
}

void reset() {
  proto_tx::reset();
  proto_rx::stop();
  proto_util::is_panic_mode = false;


//  current_parser_id = PARSER_VERSION_NUMBER;
//  current_event = EVENT_NONE;
}

void loop() {
  proto_tx::loop();
  proto_rx::loop();
}

void dumpState() {
  debug.printf("dumpState TBD");
}

//void dumpState() {
//  debug.printf(">>> parser=%d, event=%d, login_bytes=%u\n", current_parser_id, current_event, login_response_parser::bytesRead());
//}
}  // namespace proto_rx
