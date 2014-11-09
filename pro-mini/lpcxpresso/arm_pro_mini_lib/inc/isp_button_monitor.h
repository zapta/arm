#ifndef ISP_BUTTON_MONITOR_H
#define ISP_BUTTON_MONITOR_H

#include "arm_pro_mini.h"

// Monitors the ISP button and jumps to ISP mode when pressed.
namespace isp_button_monitor {

// Call once during initialization.
extern void setup();

// Call from main loop to check the ISP button and jump to
// ISP mode if pressed.
extern void loop();

}  // namespace isp_button_monitor

#endif  // ISP_BUTTON_MONITOR_H
