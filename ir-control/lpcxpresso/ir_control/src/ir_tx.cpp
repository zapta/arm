
#include "ir_tx.h"
#include "common.h"

//#include "mbed.h"
//#include "USBSerial.h"

// For ir timer
#include "cmsis.h"
#include "pinmap.h"

namespace ir_tx {

// Ticker for generating continuous 600usec interval IRQ
// for IR ticks.
Ticker ticker;

#ifndef TARGET_LPC11U35_501
#error "Should verify MCU compatibility"
#endif

static const uint16_t kCarrierFrequency = 40000;

// Map tone frequency to timer half cycle count. Since we have enough timer.
// The -1 is from the timer specification.
static const uint16_t kTimerCarrierCount =  (uint16_t)((SystemCoreClock / (kCarrierFrequency) / 2) - 1);

enum TickerState {
  IDLE,
  START,
  PRE_BIT_SPACE,
  BIT_DATA,
  POST_PACKET_SPACE
};

static TickerState ticker_state = IDLE;

// Valid in states START, BIT_DATA.
static int32_t ticks_left_in_state = 0;

// Valid in START, BIT_SPACE, BIT_DATA, PACKET_SPACE.
static int32_t ticks_from_packet_start = 0;

static int bits_left_in_packet = 0;

// The packet to send to turn my Sony audio system on/off.
static const uint32_t kDataBits = 0x540a;

// Number of bits to send from kDataBits. Only the N LSB bits of
// data_bits are transmitted with the more significant bits sent first.
static const int kDataBitsCount = 15;

// Number of errors detected so far during the IRQ. Non zero value
// indicates a software bug.
static int volatile ticker_error_count = 0;

// Number of packets left to send. Decremented after each packet sent
// until zero.
static int volatile packets_left = 0;


#define TCR_OFF    0b00
#define TCR_EN     0b01
#define TCR_RESET  0b10

void ir_setup() {
  // Disable timers 0, 1
  LPC_CT16B0->TCR = TCR_OFF;

  //Power timers 0, 1
  LPC_SYSCON->SYSAHBCLKCTRL |= 1 << (7 + 0);

  // Enable counter mode (non PWM) for timers 0, 1
  LPC_CT16B0->PWMC = 0b0000;

  // Reset functionality on MR0 controlling the counter time period
  LPC_CT16B0->MCR = (1 << 1);  // Timer0: reset counter 0 o0 MR0 match

  // Set prescaler to 1. Timer is incremented 'SystemCoreClock' times a second.
  LPC_CT16B0->PR = 0;

  // TODO: is this reset needed?
  LPC_CT16B0->TCR = TCR_RESET;

  // Clear output on match (tone are off, keep outputs  low).
  LPC_CT16B0->EMR = (0b01 << 4);  // Timer0: output LOW on MR0 match

  // Set frequency.
  LPC_CT16B0->MR0 = kTimerCarrierCount;

  LPC_CT16B0->TCR = TCR_EN;

  // Pinout
   // TODO: define a const for the PWM pin function 2.
   pin_function(P0_8, 2);  // CT16B0_MAT0
   pin_mode(P0_8, PullNone);
 }

static inline void ir_on() {
  LPC_CT16B0->EMR = (0b11 << 4);
}

static inline void ir_off() {
    LPC_CT16B0->EMR = (0b01 << 4);
}

// Should be called from the ticker IRQ only.
static void inline irq_enter_idle_state() {
  ticker_state = IDLE;
  ir_off();
  packets_left = 0;
}

// Should be called from the ticker IRQ only.
static void inline irq_enter_start_state() {
  ticker_state = START;
  ir_on();
  // Length of the start marker in ticks.
  ticks_left_in_state = 4;
  bits_left_in_packet = kDataBitsCount;
  ticks_from_packet_start = 0;
}

// Should be called from the ticker IRQ only.
static void inline irq_enter_pre_bit_space_state() {
  ticker_state = PRE_BIT_SPACE;
  ir_off();
}

// Should be called from the ticker IRQ only.
static void inline irq_enter_bit_data_state(bool bit_value) {
  ticker_state = BIT_DATA;
  ir_on();
  ticks_left_in_state = bit_value ? 2 : 1;
}

// Should be called from the ticker IRQ only.
static void inline irq_enter_post_packet_space_state() {
  ticker_state = POST_PACKET_SPACE;
  ir_off();
}

static void irq_handler() {
  switch (ticker_state) {
    case IDLE:
      if (packets_left > 0) {
        irq_enter_start_state();
      }
      return;

    case START:
      ticks_from_packet_start++;
      if ((--ticks_left_in_state) <= 0) {
        irq_enter_pre_bit_space_state();
        return;
      }
      return;

    case PRE_BIT_SPACE: {
      ticks_from_packet_start++;
      // Here always bits_left > 0.
      const bool bit_value = kDataBits &  (1 << (--bits_left_in_packet));
      irq_enter_bit_data_state(bit_value);
      return;
    }

    case BIT_DATA:
      ticks_from_packet_start++;
      if ((--ticks_left_in_state) > 0) {
        return;
      }
      // If has more bits switch to DATA_SPACE
      if (bits_left_in_packet > 0) {
        irq_enter_pre_bit_space_state();
        return;
      }
      // Else, packet done. Switch to POST_SPACE.
      irq_enter_post_packet_space_state();
      return;

    case POST_PACKET_SPACE:
      ticks_from_packet_start++;
      // Next packet can start 45 ms (== 75 600usec ticks) since start of
      // previous packet.
      if (ticks_from_packet_start < 75) {
        return;
      }
      // If still transmitting, start next packet.
      if (--packets_left > 0) {
        irq_enter_start_state();
        return;
      }
      // Else, switch to IDLE state. IR is already off.
      irq_enter_idle_state();
      return;

    // Handle unexpected state.
    default:
      ticker_error_count++;
      irq_enter_idle_state();
      return;
  }
}

void start_tx(int packets) {
  PRINTF("Start TX: %d packets\r\n", packets);
  __disable_irq();
  // TODO: should we verify that packets_left is zero now?
  packets_left = packets;
  __enable_irq();
}

int tx_packets_pending() {
  int result;

  __disable_irq();
  result = packets_left;
  __enable_irq();

  return result;
}

void dump_state() {
  __disable_irq();
  const int errors = ticker_error_count;
  __enable_irq();

  if (errors) {
    PRINTF("IR: errors=%d\r\n", errors);
  }
}

void setup() {
  ir_setup();
  ticker.attach_us(&irq_handler, 600);
}



}  // namespace ir_tx
