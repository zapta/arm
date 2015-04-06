// A special byte fifo for the serial rx. The byte pushing
// is done from an irq while the pulling is done from main.

#ifndef BYTE_FIFO_RX_H
#define BYTE_FIFO_RX_H

#include "mbed.h"

#if defined(TARGET_LPC11U35_401) || defined(TARGET_LPC11U35_501)
  static const IRQn_Type kUartIrqN = UART_IRQn;
#else
  #error "Unexpected target MCU"
#endif


class ByteFifoRxIrq {
public:
    ByteFifoRxIrq(uint8_t* buffer, uint32_t buffer_size)
      : _buffer(buffer),
        _capacity(buffer_size) {
      reset();
    }

  // Called from main.
  void reset() {
    NVIC_DisableIRQ(kUartIrqN);
    {
      _byte_count = 0;
      _first_byte_index = 0;
    }
    NVIC_EnableIRQ(kUartIrqN);
  }

//  // Called from main.
//  uint32_t size() {
//    uint32_t result ;
//    NVIC_DisableIRQ(kUartIrqN);
//    {
//      result = _byte_count;
//    }
//    NVIC_EnableIRQ(kUartIrqN);
//
//    return result;
//  }

//  // Called from main.
//  uint32_t capacity() {
//    // This is a const, no need to protect from IRQ.
//    return _capacity;
//  }

  // Returns false if a byte is not available.
  // Called from main.
  bool getByte(uint8_t* byte) {
    bool result;
    NVIC_DisableIRQ(kUartIrqN);
    {
      if (!_byte_count) {
        result = false;
      } else {
        *byte = _buffer[_first_byte_index++];
        if (_first_byte_index >= _capacity) {
          _first_byte_index = 0;
        }
        _byte_count--;
        result = true;
      }
    }
    NVIC_EnableIRQ(kUartIrqN);

    return result;
  }

  // Return false if buffer was full.
  // Called from RX interrupt.
  bool putByteInIrq(uint8_t byte) {
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

#endif  // BYTE_FIFO_RX_H
