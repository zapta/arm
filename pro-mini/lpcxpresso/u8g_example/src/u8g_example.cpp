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

#include <stdlib.h>
#include <stdio.h>

// The base arm_pro_mini_lib include.
#include "arm_pro_mini.h"

// Provides abstraction to digital I/o pins.
#include "io_pins.h"

// Provide system time using TIMER32 0.
#include "system_time.h"

// Provides interrupt free elapsed time measurement using system time.
#include "passive_timer.h"

// For serial output.
#include "usb_serial.h"

// The ARM PRO MINI specific driver of u8glib. This also includes the
// standard u8g.h.
#include "u8g_arm_pro_mini.h"

// Red LED is at GPIO0_7.
static io_pins::OutputPin led(0, 7);

// Timer timing the renderings.
static PassiveTimer timer;

// The U8G instance (a C struct).
static u8g_t u8g;

// Screen dimensions,
// x range = [0, kMaxX]
// y range = [0, kMaxY]
static const int kMaxX = 128 - 1;
static const int kMaxY = 64 - 1;

static const int kSpeed = 1;

// Return a random number in [min, max].
static inline int randRange(int min, int max) {
  return min + rand() % (max - min + 1);
}

// An up/down counter bouncing between [min, max].
struct BouncingValue {
  BouncingValue(int min, int max) :
      min(min),
      max(max),
      value(randRange(min, max)),
      speed(kSpeed) {
  }

  void step() {
    // Handle edge bounce
    if ((speed > 0 && value >= max) || (speed < 0 && value <= min)) {
      speed = -speed;
    }
    // Move by one unit of time.
    value += speed;
    // Clip to min/max.
    if (value < min) {
      value = min;
    } else if (value > max) {
      value = max;
    }
  }

  const int min;
  const int max;
  int value;
  int speed;
};

// A ball of given radius bouncing on x and y ranges.
struct Ball {
  Ball(int r) :
      r(r), x(r, kMaxX - r), y(r, kMaxY - r) {
  }

  void step() {
    x.step();
    y.step();
  }

  void draw() {
    u8g_DrawDisc(&u8g, x.value, y.value, r, U8G_DRAW_ALL);
    u8g_DrawLine(&u8g, 0, 0, x.value, y.value);
    u8g_DrawLine(&u8g, 0, kMaxY, x.value, y.value);
    u8g_DrawLine(&u8g, kMaxX, 0, x.value, y.value);
    u8g_DrawLine(&u8g, kMaxX, kMaxY, x.value, y.value);

    char bfr[15];
    snprintf(bfr, sizeof(bfr), "x:%d y:%d", x.value, y.value);
    u8g_DrawStr(&u8g, 38, kMaxY-9, bfr);
  }

  const int r;
  BouncingValue x;
  BouncingValue y;
};

// Represents the display state and operations.
struct DisplayModel {
  DisplayModel() : ball(3) {
  }

  void draw() {
    // Prepare. Could be move to setup().
    u8g_SetFont(&u8g, u8g_font_6x10);
    u8g_SetFontRefHeightExtendedText(&u8g);
    u8g_SetDefaultForegroundColor(&u8g);
    u8g_SetFontPosTop(&u8g);

    u8g_DrawStr(&u8g, kMaxX/2-32, 0, "ARM PRO MINI");
    ball.draw();
  }

  void step() {
    ball.step();
  }

  Ball ball;
};

DisplayModel displayModel;

// One time initialization of the board.
static void setup() {
  arm_pro_mini::setup();

  // Uses timer32_0 for generating a 1 usec 32 bit clock (does not use interrupts).
  system_time::setup();

  // Reset the timer to the time now. This starts the first cycle.
  timer.reset();

  usb_serial::setup();

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
      displayModel.draw();
    } while (u8g_NextPage(&u8g));

    displayModel.step();
    usb_serial::printf("x:%d, y:%d\r\n",
        displayModel.ball.x.value,
        displayModel.ball.y.value);
  }
}

int main() {
  setup();
  for (;;) {
    loop();
  }
}
