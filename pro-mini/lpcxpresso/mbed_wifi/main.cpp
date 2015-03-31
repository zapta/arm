#include "mbed.h"
#include <string.h>
#include <ctype.h>
#include "esp8266.h"
#include "debug.h"
// TODO: hide protobuff.h behind protocol.h
#include "proto.h"
//#include "protocol.h"
#include "_config.h"
//#include "parser.h"

static DigitalOut led(P0_20, 0);

static Timer timer;

static Timer led_timer;

static void setup() {
  timer.start();
  led_timer.start();
  esp8266::setup();

  proto::setup();
  //protocol::setup();
  //parser::setup();
}

enum MainState {
  NOT_CONNECTED,
  CONNECTION_START,
  WAIT_LOGIN_RESPONSE,
  CONNECTED,
  CONNECTION_END,
};

static MainState state = NOT_CONNECTED;
static uint32_t last_connection_id = -1;

//static void test_serializer_varint(uint64_t v) {
//  debug.printf("\n--- %08x %08x\n", (uint32_t)(v >> 32), (uint32_t)(v & 0xffffffff));
//  serializer::writeVarint(v);
//}
//
//static void test_serializer_varint_tag(uint64_t tag_num) {
//  debug.printf("\n--- varint tag %0x\n", tag_num);
//  serializer::writeVarintTag(tag_num);
//}

//static void testLoginRequest() {
//  debug.printf("\n\n********\n");
//  protocol::sendProtocolVersionByte();
//  protocol::sendLoginRequestAndFlush("id", "domain", "user");
//}

//static char bfr[20+1];
//static char* uint64ToDecimal(uint64_t v) {
//  int p = sizeof(bfr);
//  bfr[--p] = '\0';
//  for (bool first = true; v || first; first = false) {
//    uint32_t digit = v % 10;
//    const char c = '0' + digit;
//    bfr[--p] = c;
//    v = v / 10;
//  }
//  return &bfr[p];
//}
//
//static char* uint64ToHex(uint64_t v) {
//  int p = sizeof(bfr);
//  bfr[--p] = '\0';
//  for (bool first = true; v || first; first = false) {
//    uint32_t digit = v & 0xf;
//    const char c = (digit < 10)
//        ? ('0' + digit)
//        : ('a' + digit - 10);
//    bfr[--p] = c;
//    v = v >> 4;
//  }
//  return &bfr[p];
//}

static void loop() {
//  for(;;) {
//    //atoll()
//    //static char bfr[100];
//    //uint64_t value = -3;
//    //snprintf(bfr, sizeof(bfr), "%llu", value);
//    debug.printf("[%s]\n", uint64ToHex(0x1198abcdef8llu));
//    wait_ms(1000);
//  }

  esp8266::loop();
  proto::loop();
  //parser::loop();
  //protocol::loop();

  led.write(led_timer.read_ms() < 200);
  if (led_timer.read_ms() >= 3000) {
    led_timer.reset();
    esp8266::dumpState();
    //protocol::dumpState();
    debug.printf("**: %d\n", state);
  }

  const uint32_t connection_id = esp8266::connectionId();

  switch (state) {
    case NOT_CONNECTED:
      // TODO: clear the proto buffer buffers.
      if (connection_id) {
        debug.printf("## CONN START\n");
        state = CONNECTION_START;
        last_connection_id = connection_id;
      }
      break;

    case CONNECTION_START:
      proto_rx::start();
      //protocol::reset();
      debug.printf("## SENDING LOGIN\n");
      proto_tx::sendProtocolVersion();
      proto_tx::sendLoginRequest(config::device_id, config::auth_token);
      state = WAIT_LOGIN_RESPONSE;
      break;

//    case WAIT_PROTOCOL_VERSION: {
//      const protocol::EventType event = protocol::currentEvent();
//      if (event) {
//        if (event == protocol::EVENT_PROTOCOL_VERSION) {
//          debug.printf("Remote protocol version %u\n", protocol::protocol_version_event.version);
//          protocol::nextEvent();
//          state = WAIT_LOGIN_RESPONSE;
//        } else {
//          // TODO: have this method in protocl::, do not user protobuf direclty.
//          protobuf::protocolPanic("expecting login resp");
//        }
//      }
//    }
      break;

    case WAIT_LOGIN_RESPONSE:
         // TODO: implement
      state = CONNECTED;
         break;

    case CONNECTED:
      if (connection_id != last_connection_id) {
        state = CONNECTION_END;
      }
      break;

    case CONNECTION_END:
      proto::reset();
//
//      debug.printf("*** TRAP, STOPPING\n");
//      for(;;){}
//
      state = NOT_CONNECTED;
      break;

    default:
      debug.printf("Unknown: %d\n", state);
      state = NOT_CONNECTED;
  }
}

int main(void) {
  setup();
  for (;;) {
    loop();
  }
}
