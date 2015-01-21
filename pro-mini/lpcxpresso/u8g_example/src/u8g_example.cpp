// A sample ARM PRO MINI program with u8g demo.
//
// Using Heltec's 128x64 1.3" white module with ssd1306 controller. Note
// that similar Heltec modules with different pin labels or with blue
// display are different and requires adaptation.
//
// NOTE: the g8glib reference manual is available here
// https://code.google.com/p/u8glib/wiki/userreference
//
// NOTE: the definitive pin assignment of the SPI OLED driver
// is in u8g_arm_pro_mini.cpp
//
// GND - pin 28. Ground.
// VCC - pin 27. 3.3V
// SCL - pin 24. SPI SCK 0.
// SDA - pin  5. SPI MOSI 0.
// RST - pin 16. P0_18 (reset).
// D/C - pin 18. P0_19 (a0).

// The base arm_pro_mini_lib include.
#include "arm_pro_mini.h"

// Provides abstraction to digital I/o pins.
#include "io_pins.h"

// Provide system time using TIMER32 0.
#include "system_time.h"

// Provides interrupt free elapsed time measurement using system time.
#include "passive_timer.h"

// The ARM PRO MINI specific driver of u8glib. This also includes the
// standard u8g.h.
#include "u8g_arm_pro_mini.h"

// Red LED is at GPIO0_7.
static io_pins::OutputPin led(0, 7);

// Timer timing the renderings.
static PassiveTimer timer;

// The U8G instance (a C struct).
u8g_t u8g;

static uint8 drawing_state = 0;

static void draw(void) {
  // Prepare. Could be move to setup().
  u8g_SetFont(&u8g, u8g_font_6x10);
  u8g_SetFontRefHeightExtendedText(&u8g);
  u8g_SetDefaultForegroundColor(&u8g);
  u8g_SetFontPosTop(&u8g);

  // Draw lines.
  u8g_DrawStr(&u8g, 0, 0, "Draw lines");
  u8g_DrawLine(&u8g, drawing_state, 10, 40, 63);
  u8g_DrawLine(&u8g, drawing_state * 2, 10, 60, 63);
  u8g_DrawLine(&u8g, drawing_state * 3, 10, 80, 63);
  u8g_DrawLine(&u8g, drawing_state * 4, 10, 100, 63);
}

// One time initialization.
static void setup() {
  arm_pro_mini::setup();

  // Uses timer32_0 for generating a 1 usec 32 bit clock (does not use interrupts).
  system_time::setup();

  // Reset the timer to the time now. This starts the first cycle.
  timer.reset();

  // u8g initialization for the ssd1306 128x64 oled we use with SPI0.
  u8g_InitComFn(&u8g, &u8g_dev_ssd1306_128x64_hw_spi, u8g_com_hw_spi_fn);
}

// Main loop
static void loop() {
  const uint32 usecs_in_cycle = timer.usecs();

  if (usecs_in_cycle >= 50000L) {

    led.set(!led.get());

    timer.reset();

    // U8G picture loop. Each loop renders the next portion
    // of the display (aka 'page'). This sacrifices CPU time to save display
    // buffer memory.
    //
    // TODO: to reduce max loop time draw each page in a separate
    // call to loop(). Hopefully it will not cause flicker.
    //
    // This loop iterates 8 times with about 1ms per iteration.
    u8g_FirstPage(&u8g);
    do {
      draw();
    } while (u8g_NextPage(&u8g));

    // Note that we change the drawing state after each full screen
    // drawing, not after each drawing of a portion of the screen.
    if  (++drawing_state >= 32) {
      drawing_state = 0;
    }
  }
}

int main() {
  setup();
  for (;;) {
    loop();
  }
}
