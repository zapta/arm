// A sample ARM PRO MINI program with u8g demo.
//
// Using Heltec's 128x64 1.3" white module with ssd1306 controller. Note
// that similar Heltec modules with different pin labels or with blue
// display are different and requires adaptation.
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

// Allows to jump to ISP mode when ISP button is pressed.
#include "isp_button_monitor.h"

#include "u8g_arm_pro_mini.h"

// Red LED is at GPIO0_7.
static io_pins::OutputPin led(0, 7);

// Timer timing the renderings.
static PassiveTimer timer;

// The U8G instance.
//
// TODO: expose the u8g instance as a class instance with methods rather
// c functions.
u8g_t u8g;

static void u8g_prepare(void) {
  u8g_SetFont(&u8g, u8g_font_6x10);
  u8g_SetFontRefHeightExtendedText(&u8g);
  u8g_SetDefaultForegroundColor(&u8g);
  u8g_SetFontPosTop(&u8g);
}

static void u8g_box_frame(uint8 a) {
  u8g_DrawStr(&u8g, 0, 0, "drawBox");
  u8g_DrawBox(&u8g, 5, 10, 20, 10);
  u8g_DrawBox(&u8g, 10 + a, 15, 30, 7);
  u8g_DrawStr(&u8g, 0, 30, "drawFrame");
  u8g_DrawFrame(&u8g, 5, 10 + 30, 20, 10);
  u8g_DrawFrame(&u8g, 10 + a, 15 + 30, 30, 7);
}

static void u8g_string(uint8 a) {
  u8g_DrawStr(&u8g, 30 + a, 31, " 0");
  u8g_DrawStr90(&u8g, 30, 31 + a, " 90");
  u8g_DrawStr180(&u8g, 30 - a, 31, " 180");
  u8g_DrawStr270(&u8g, 30, 31 - a, " 270");
}

static void u8g_line(uint8 a) {
  u8g_DrawStr(&u8g, 0, 0, "drawLine");
  u8g_DrawLine(&u8g, 7 + a, 10, 40, 55);
  u8g_DrawLine(&u8g, 7 + a * 2, 10, 60, 55);
  u8g_DrawLine(&u8g, 7 + a * 3, 10, 80, 55);
  u8g_DrawLine(&u8g, 7 + a * 4, 10, 100, 55);
}

static void u8g_ascii_1(void) {
  char s[2] = " ";
  uint8 x, y;
  u8g_DrawStr(&u8g, 0, 0, "ASCII page 1");
  for (y = 0; y < 6; y++) {
    for (x = 0; x < 16; x++) {
      s[0] = y * 16 + x + 32;
      u8g_DrawStr(&u8g, x * 7, y * 10 + 10, s);
    }
  }
}

static void u8g_ascii_2(void) {
  char s[2] = " ";
  uint8 x, y;
  u8g_DrawStr(&u8g, 0, 0, "ASCII page 2");
  for (y = 0; y < 6; y++) {
    for (x = 0; x < 16; x++) {
      s[0] = y * 16 + x + 160;
      u8g_DrawStr(&u8g, x * 7, y * 10 + 10, s);
    }
  }
}

static uint8 draw_state = 0;

static void draw(void) {
  u8g_prepare();
  switch (draw_state >> 3) {
  case 0:
    u8g_box_frame(draw_state & 7);
    break;
  case 1:
    u8g_string(draw_state & 7);
    break;
  case 2:
    u8g_line(draw_state & 7);
    break;
  case 3:
    u8g_ascii_1();
    break;
  case 4:
    u8g_ascii_2();
    break;
  }
}

// One time initialization.
static void setup() {
  arm_pro_mini::setup();

  // Uses timer32_0 for generating a 1 usec 32 bit clock (does not use interrupts).
  system_time::setup();

  // Reset the timer to the time now. This starts the first cycle.
  timer.reset();

  // Get ready to monitor the ISP button
  isp_button_monitor::setup();

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
    // TODO: to reduce max loop time draw each page in a seperate
    // call to loop(). Hopefully it will not cause flicker.
    //
    // This loop iterates 8 times with about 1ms per iteration.
    u8g_FirstPage(&u8g);
    do {
      draw();
    } while (u8g_NextPage(&u8g));

    draw_state++;
    if (draw_state >= 5 * 8) {
      draw_state = 0;
    }
  }
}

int main() {
  setup();
  for (;;) {
    loop();
  }
}
