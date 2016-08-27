#include "util/status_led.h"

namespace status_led {

// LED, active high.
DigitalOut led_pin(P0_20, 0);

}  // namespace status_led
