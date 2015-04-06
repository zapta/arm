#ifndef STRING_POOL_H
#define STRING_POOL_H

#include "mbed.h"

class StringPool {
public:
  StringPool(char* buffer, uint32_t buffer_size)
    : _buffer(buffer),
      _buffer_size(buffer_size) {
    reset();
  }

  void reset() {
    _current_string = NULL;
    _total_bytes_used = 0;
  }

  const char* createNewString() {
    if (_total_bytes_used >= _buffer_size) {
      // Buffer is full return a pointer to an empty string that cannot grow.
      _current_string = "";
    } else {
      _current_string = &_buffer[_total_bytes_used];
      _buffer[_total_bytes_used++] = '\0';
    }
    return _current_string;
  }


  bool addByteToLastString(char c) {
    // Do nothing if no string.
    // TODO: this is a programming error. Consider a more string handling.
    if (!_current_string) {
      return false;
    }

    // No room for more bytes.
    if (_total_bytes_used >= _buffer_size) {
      return false;
    }

    // Replace the null terminator with the new char and append a
    // new null terminator.
    _buffer[_total_bytes_used-1] = c;
    _buffer[_total_bytes_used++] = '\0';
    return true;
  }

  int freeSpace() {
    return _buffer_size - _total_bytes_used;
  }

protected:
  char* const _buffer;
  const uint32_t _buffer_size;
  const char* _current_string;
  uint32_t _total_bytes_used;
};

#endif  // #ifndef STRING_POOL_H

