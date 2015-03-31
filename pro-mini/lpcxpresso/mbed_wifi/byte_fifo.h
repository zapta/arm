#ifndef BYTE_FIFO_H
#define BYTE_FIFO_H

#include "mbed.h"

class ByteFifo {
public:
  ByteFifo(uint8_t* buffer, uint32_t buffer_size)
    : _buffer(buffer),
      _max_buffer_size(buffer_size) {
    //_bytes_consumed = 0;
    reset();
  }

  void reset() {
    _byte_count = 0;
    _first_byte_index = 0;
    // _bytes_read is not affected by reset();
  }

  inline bool isFull() {
    return _byte_count >= _max_buffer_size;
  }

  inline uint32_t size() {
    return _byte_count;
  }

  inline uint32_t maxSize() {
    return _max_buffer_size;
  }

  // Returns false if a byte is not available.
  bool getByte(uint8_t* byte) {
    if (!_byte_count) {
      return false;
    }
    *byte = _buffer[_first_byte_index++];
    if (_first_byte_index >= _max_buffer_size) {
      _first_byte_index = 0;
    }
    _byte_count--;
    //_bytes_consumed++;
    return true;
  }

  // TODO: remove the peek method. Not needed anymore

//  bool peekByte(uint32_t index, uint8_t* byte) {
//    if (index >= _byte_count) {
//      return false;
//    }
//    uint32_t byte_index = _first_byte_index + index;
//    if (byte_index >= _max_buffer_size) {
//      byte_index -= _max_buffer_size;
//    }
//    *byte = _buffer[byte_index];
//    return true;
//  }

//  // Skip first n bytes. If fifo has less than n bytes then empty the buffer and
//  // return false.
//  bool skipBytes(unsigned n) {
//    if (n > _byte_count) {
//      _bytes_consumed += _byte_count;
//      reset();
//      return false;
//    }
//    _byte_count -= n;
//    _first_byte_index += n;
//    _bytes_consumed += n;
//    if (_first_byte_index >= _max_buffer_size) {
//      _first_byte_index -= _max_buffer_size;
//    }
//    return true;
//  }

  // Return false if buffer was full.
  bool putByte(uint8_t byte) {
    if (_byte_count >= _max_buffer_size) {
      return false;
    }
    uint32_t insertion_index = _first_byte_index + _byte_count;
    if (insertion_index >= _max_buffer_size) {
      insertion_index -= _max_buffer_size;
    }
    _buffer[insertion_index] = byte;
    _byte_count++;

    return true;
  }

//  uint32_t bytesConsumed() {
//    return _bytes_consumed;
//  }

private:
  uint8_t* const _buffer;
  const uint32_t _max_buffer_size;
  uint32_t _byte_count;
  uint32_t _first_byte_index;

  // @@@@ TODO: delete _bytes_consumed field. Not needed.
  // Not affected by reset().
  //uint32_t _bytes_consumed;
};

#endif  // BYTE_FIFO_H
