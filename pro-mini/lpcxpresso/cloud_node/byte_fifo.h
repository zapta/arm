#ifndef BYTE_FIFO_H
#define BYTE_FIFO_H

#include "mbed.h"

class ByteFifo {
public:
  ByteFifo(uint8_t* buffer, uint32_t buffer_size)
    : _buffer(buffer),
      _capacity(buffer_size) {
    reset();
  }

  void reset() {
    _byte_count = 0;
    _first_byte_index = 0;
  }

//  bool isFull() {
//    return _byte_count >= _capacity;
//  }

  uint32_t size() {
    return _byte_count;
  }

//  uint32_t capacity() {
//    return _capacity;
//  }

  // Returns false if a byte is not available.
  bool getByte(uint8_t* byte) {
    if (!_byte_count) {
      return false;
    }
    *byte = _buffer[_first_byte_index++];
    if (_first_byte_index >= _capacity) {
      _first_byte_index = 0;
    }
    _byte_count--;
    return true;
  }

  // Return false if buffer was full.
  bool putByte(uint8_t byte) {
    if (_byte_count >= _capacity) {
      return false;
    }
    uint32_t insertion_index = _first_byte_index + _byte_count;
    if (insertion_index >= _capacity) {
      insertion_index -= _capacity;
    }
    _buffer[insertion_index] = byte;
    _byte_count++;

    return true;
  }

protected:
  uint8_t* const _buffer;
  const uint32_t _capacity;
  uint32_t _byte_count;
  uint32_t _first_byte_index;
};

#endif  // BYTE_FIFO_H
