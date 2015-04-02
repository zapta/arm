
#include "protocol_tx.h"
#include "debug.h"
#include "esp8266.h"
#include "protocol_util.h"

namespace protocol_tx {

static const char kDomain[] = "mcs.android.com";

static char tmp_buffer[30];

// Size is sufficient to contain a uint64 in decimal or hex format including
// a null terminator.
static char uint64_to_str_buffer[20+1];

static char* uint64ToDecimal(uint64_t v) {
  char* p = uint64_to_str_buffer + sizeof(uint64_to_str_buffer);
  *(--p) = '\0';
  for (bool first = true; v || first; first = false) {
    uint32_t digit = v % 10;
    const char c = '0' + digit;
    *(--p) = c;
    v = v / 10;
  }
  return p;
}

static char* uint64ToHex(uint64_t v) {
  char* p = uint64_to_str_buffer + sizeof(uint64_to_str_buffer);
  *(--p) = '\0';
  for (bool first = true; v || first; first = false) {
    uint32_t digit = v & 0xf;
    const char c = (digit < 10)
        ? ('0' + digit)
        : ('a' - 10 + digit);
    *(--p) = c;
    v = v >> 4;
  }
  return p;
}

// Total bytes written so far.
static uint32_t total_bytes_written = 0;

// When true, we don't actually send out data, just incrementing total_bytes_written.
// Useful to determine message size before acutally sending it in a second pass.
static bool is_counting_only_mode = false;

// The lowest level byte serialization.
void writeRawByte(uint8_t b) {
  if (!is_counting_only_mode) {
    if (!esp8266::tx_fifo.putByte(b)) {
      protocol_util::protocolPanic("TX ovf");
      return;
    }
    debug.printf("TX[%02x]\n", b);
  }
  total_bytes_written++;
}

// Send given bytes.
static void sendProtocolRawBytes(const uint8_t* data, uint32_t size) {
  while (size--) {
    writeRawByte(*data++);
  }
}

// Serialize a varint.
void writeVarint(uint64_t value) {
  while (value > 0x7F) {
    writeRawByte((static_cast<uint8_t>(value) & 0x7F) | 0x80);
    value >>= 7;
  }
  writeRawByte(static_cast<uint8_t>(value) & 0x7F);
}

// Serialize a field tag.
void writeTag(uint32_t tag_num, uint8_t tag_type) {
  writeVarint(tag_num << 3 | tag_type);
}

// Serialize a varint field with given tag. Can be used to serialize
// int enums and bool fields.
void writeVarintField(uint32_t tag_num, uint64_t value) {
  writeTag(tag_num, protocol_util::kTagTypeVarint);
  writeVarint(value);
}

// Serialize a bool field of given tag.
void writeBoolField(uint32_t tag_num, bool value) {
  writeVarintField(tag_num, value ? 1 : 0);
}

// Serialize a string field. Per the protocol buffers encoding specification
// the null char terminator is not written out.
void writeStringField(uint32_t tag_num, const char* value) {
  writeTag(tag_num, protocol_util::kTagTypeLenDelimited);
  const uint32_t n = strlen(value);
  writeVarint(n);
  sendProtocolRawBytes(reinterpret_cast<const uint8_t*>(value), n);
}


void sendProtocolVersion() {
  is_counting_only_mode = false;
  writeVarint(38);
}

void sendLoginRequest(uint64_t device_id, uint64_t auth_token) {
  debug.printf("id=%08x:%08x\n", static_cast<uint32_t>(device_id >> 32),
      static_cast<uint32_t>(device_id));
  debug.printf("auth=%08x:%08x\n", static_cast<uint32_t>(auth_token >> 32),
      static_cast<uint32_t>(auth_token));

  uint32_t pass0_size = 0;
  uint32_t pass1_size = 0;

  is_counting_only_mode = true;

  for (int pass = 0; pass < 2; pass++) {
    const uint32_t pass_start_count = total_bytes_written;

    writeStringField(1, "");  // Not used. Required field.
    writeStringField(2, kDomain);  // domain

    const char* t = uint64ToDecimal(device_id);
    writeStringField(3, t);  // user
    writeStringField(4, t);  // resource

    t = uint64ToDecimal(auth_token);
    writeStringField(5, t);  // auth token

    t = uint64ToHex(device_id);
    snprintf(tmp_buffer, sizeof(tmp_buffer), "android-%s", t);
    writeStringField(6, tmp_buffer);  // device_id

    writeVarintField(16, 2);  // auth_service = ANDROID_ID

    if (pass == 0) {
      pass0_size = total_bytes_written - pass_start_count;
      is_counting_only_mode = false;
      // Send message header
      writeVarint(2);  // message tag for LoginRequest.
      writeVarint(pass0_size);
    } else {
      // This else section is for verification only.
      pass1_size = total_bytes_written - pass_start_count;
    }
  }
  debug.printf("LoginRequest sent, %u (%u) bytes\n", pass1_size, pass0_size);
}

void sendHeartbeatPing() {
  uint32_t pass0_size = 0;
  uint32_t pass1_size = 0;

  is_counting_only_mode = true;
  for (int pass = 0; pass < 2; pass++) {
    const uint32_t pass_start_count = total_bytes_written;

    // TODO: send actual values.
    writeVarintField(1, 1);  // Last ocal stream id sent.
    writeVarintField(2, 1);  // Last remote stream id recieved.
    writeVarintField(3, 1);  // Status

    if (pass == 0) {
      pass0_size = total_bytes_written - pass_start_count;
      is_counting_only_mode = false;
      writeVarint(0);  // message tag for LoginRequest.
      writeVarint(pass0_size);
    } else {
      // This else section is for verification only.
      pass1_size = total_bytes_written - pass_start_count;
    }
  }
  debug.printf("HeartbeatPing sent, %u (%u) bytes\n", pass1_size, pass0_size);
}

void setup() {
  reset();
}

void loop() {
  // Nothing to do.
}

void reset() {
  total_bytes_written = 0;
  is_counting_only_mode = false;
  esp8266::rx_fifo.reset();
}

}  // namespace protocol_tx
