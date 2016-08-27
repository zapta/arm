
#ifndef WIFI_WIFI_PARSER_H
#define WIFI_WIFI_PARSER_H

#include "mbed.h"
#include "util/byte_fifo.h"


namespace wifi_parser {

// ----- Housekeeping

extern void initialize();
extern void polling();
extern void dumpInternalState();

// Indicates how to parse the response.
enum ResponseParsingMode {
  // This is a standard response, includes a header and a text data that is
  // left in the parser's line buffer.
  STD,

  // This is a response of a client list command.
  CLIENT_LIST
};

// Should be called just before sending the command to the wifi module.
extern void startParsingNextResponse(ResponseParsingMode response_parsing_mode);

extern bool isResponseParsingDone();

extern char line_buffer[];
extern unsigned int line_buffer_size;


}  // namespace wifi_parser

#endif  // WIFI_WIFI_PARSER_H
