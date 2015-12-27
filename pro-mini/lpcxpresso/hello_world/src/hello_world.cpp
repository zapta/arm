// A basic hello world example using the arm_pro_mini library. It blinks the LED
// and prints to the USB/CDC serial port.

// These #include files are from the arm_pro_mini_lib library.

// The base arm_pro_mini_lib include.
#include "arm_pro_mini.h"

// Provides abstraction to digital I/o pins.
#include "io_pins.h"

// Provide system time using the CT32B1 timer.
#include "system_time.h"

// Provides interrupt free elapsed time measurement using system time.
#include  "passive_timer.h"

// Provides serial I/O over USB/CDC.
#include "usb_serial.h"

// Analog to Digital Converter (ADC) module
#include "adc.h"

//Standard Input and Output Library.
#include "stdio.h"

// LED blink cycle. We provide two prebuilt binaries with
// fast and slow blink respectively to be used in the
// Getting Started procedure.
//
static const uint32 kCycleTimeUsecs = 100 * 1000;  // fast
// static const uint32 kCycleTimeUsecs = 2000 * 1000;  // slow

// Timer for generating the delay bettween printed messages.
static PassiveTimer timer;

// Red LED is at GPIO0_20.
static io_pins::OutputPin led1(0, 20);

// Early versions of ARM PRO MINI had the led
// at GPIO0_7.
static io_pins::OutputPin legacy_led(0, 7);

// Analog channel 0 initialize @PCB pin 7
static ADC::ADC_Init Analog0(0);

static void setup() {
	arm_pro_mini::setup();
	// Uses timer32_0 for generating a 1 usec 32 bit clock (does not use interrupts).
	system_time::setup();
	// Initialize the USB serial connection. This will allow us to print messages.
	usb_serial::setup();
	// Reset the timer to the time now. This starts the first cycle.
	timer.reset();
}

static void loop() {
	static int message_count = 0;
	const uint32 time_now_in_cycle_usecs = timer.usecs();

	// Generates a blink at the beginning of each cycle.
	const bool led_state = time_now_in_cycle_usecs <= kCycleTimeUsecs / 3;
	led1.set(led_state);
	legacy_led.set(led_state);

	if (time_now_in_cycle_usecs >= kCycleTimeUsecs) {
		// NOTE: using \r\n EOL for the benefit of dumb serial dump. Typically
		// \n is sufficient.
		printf("Hello world: %d,ADC0: %d, %lu\r\n", message_count,
				Analog0.Read(), system_time::usecs());
		message_count++;
		// Advance cycle start time rather than reseting to time now. This
		// way we don't accumulate time errors.
		timer.advance_start_time_usecs(kCycleTimeUsecs);
	}
}

int main(void) {
	setup();
	for (;;) {
		loop();
	}
}
