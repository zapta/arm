#include "isp_button_monitor.h"

#include "arm_pro_mini.h"
#include "chip.h"

namespace isp_button_monitor {

// ISP button is attached to GPIO0_1.
// TODO: define in arm_pro_mini consts for ISP button and LED ports.
static const uint8 ISP_BUTTON_PORT = 0;
static const uint8 ISP_BUTTON_BIT  = 1;

void setup() {
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, ISP_BUTTON_PORT, ISP_BUTTON_BIT);
}

void loop() {
  // The button is active low.
  if (!Chip_GPIO_ReadPortBit(LPC_GPIO, ISP_BUTTON_PORT, ISP_BUTTON_BIT)) {
    arm_pro_mini::ReinvokeISP();
  }
}

}  // namespace isp_button_monitor
