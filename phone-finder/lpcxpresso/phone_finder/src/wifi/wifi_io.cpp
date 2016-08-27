// Implementation of wifi_io.h.

#include "wifi/wifi_io.h"
#include "util/byte_fifo.h"

namespace wifi_io {

// The mbed standard Serial has a reputation of not working well with
// interrupts. Using RawSerial instead.
static RawSerial wifi_serial(P0_19, P0_18);  // tx, rx

// Serial flow control. Signals are active low.
static DigitalOut wifi_cts_out(P0_23, 0);  // connect to wifi CTS
static DigitalIn wifi_rts_in(P0_17);  // connect to wifi RTS

// connects to wifi 'button1', active high.
static DigitalOut wifi_factory_reset_out(P0_14, 0);

// connects to wifi 'button2', active high.
static DigitalOut wifi_wake_out(P0_7, 0);

// connects to wifi N_RESET, active low.
static DigitalOut wifi_reset_out(P0_11, 0);

// Connection data pending. This pin is specified with the -g option when
// opening a new connection.
static DigitalIn wifi_irq_in(P0_2);  // connect to gpio 16

// Serial input FIFO. Bytes are inserted in an interrupt routine. This
// is why we don't use the standard the byte fifo.
static uint8_t rx_buffer[200];
static uint32_t rx_byte_count;
static uint32_t rx_first_byte_index;

// Enable RTS out when the rx buffer has at lest this number of free
// bytes.
static const uint32_t kRtsThreshold = 20;

// Serial output FIFO.
static uint8_t tx_fifo_buffer[200];
static ByteFifo tx_fifo(tx_fifo_buffer, sizeof(tx_fifo_buffer));

// Indicates that rx irq dropped bytes due to a full serial_rx_irq_fifo.
static bool rx_overflow;

#if defined(TARGET_LPC11U35_401) || defined(TARGET_LPC11U35_501)
static const IRQn_Type kUartIrqN = UART_IRQn;
#else
#error "Unexpected target MCU"
#endif

// Called from RX critical section (IRQ disabled) or IRQ to update the
// RTS pin.
static void rx_update_rts_out() {
  const uint32_t free_bytes = sizeof(rx_buffer) - rx_byte_count;
  const bool enable_rts = free_bytes >= kRtsThreshold;
  wifi_cts_out.write(!enable_rts);  // active low
}

// Called from main.
void rx_reset() {
  // Critical section.
  NVIC_DisableIRQ(kUartIrqN);
  {
    rx_byte_count = 0;
    rx_first_byte_index = 0;
    rx_overflow = false;
    rx_update_rts_out();
  }
  NVIC_EnableIRQ(kUartIrqN);
}

// Returns false if a byte is not available.
// Called from main.
bool rx_read_byte(uint8_t* byte) {
  bool result;
  // Critical section
  NVIC_DisableIRQ(kUartIrqN);
  {
    if (!rx_byte_count) {
      result = false;
    } else {
      *byte = rx_buffer[rx_first_byte_index++];
      if (rx_first_byte_index >= sizeof(rx_buffer)) {
        rx_first_byte_index = 0;
      }
      rx_byte_count--;
      rx_update_rts_out();
      result = true;
    }
  }
  NVIC_EnableIRQ(kUartIrqN);

  return result;
}

// Called at interrupt level when serial has pending RX bytes.
static void rx_interrupt() {
  while ((wifi_serial.readable())) {
    const uint8_t byte = wifi_serial.getc();
    if (rx_byte_count >= sizeof(rx_buffer)) {
      rx_overflow = true;
    } else {
      uint32_t insertion_index = rx_first_byte_index + rx_byte_count;
      if (insertion_index >= sizeof(rx_buffer)) {
        insertion_index -= sizeof(rx_buffer);
      }
      rx_buffer[insertion_index] = byte;
      rx_byte_count++;
      rx_update_rts_out();
    }
  }
}

void initialize() {
  wifi_module_control(true);
  rx_reset();

  wifi_serial.baud(115200);
  wifi_serial.format(8, SerialBase::None, 1);
  wifi_serial.attach(&rx_interrupt, Serial::RxIrq);
}

void polling() {
  // Transfer bytes from the tx fifo to the serial out. We can do it
  // when the tx fifo has bytes, the serial TX can receive bytes and the
  // CTS input is low.
  while (tx_fifo.size() && wifi_serial.writeable() && !wifi_rts_in.read()) {
    uint8_t byte = 0;
    tx_fifo.getByte(&byte);  // should not fail
    wifi_serial.putc(byte);  // should not fail
  }
}

int tx_free_bytes() {
  return tx_fifo.free();
}

bool tx_is_empty() {
  return !tx_fifo.size();
}

bool tx_put_byte(uint8_t byte) {
  return tx_fifo.putByte(byte);
}

bool is_rx_overflow() {
  // NOTE: no need to disable interrupts to access this bool.
  // We just care about zero/non-zero.
  return rx_overflow;
}

void wifi_module_control(bool reset_active) {
  wifi_reset_out.write(!reset_active);  // reset is active low
  wifi_factory_reset_out.write(0);  // inactive
  wifi_wake_out.write(0);  // inactive
}

bool wifi_module_pending_stream_data() {
  return wifi_irq_in;
}

void dumpInternalState() {
  PRINTF(
      "WIFI_IO rx=%d, tx=%d, cts_in=%d, rts_out=%d\n",
      rx_byte_count, tx_fifo.size(), wifi_rts_in.read(), wifi_cts_out.read());
}

}  // namespace wifi_io

