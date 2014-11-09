
#include "arm_pro_mini.h"

#include "chip.h"
#include "chip/wwdt_11xx.h"
#include "chip/romapi_11xx.h"

/* System oscillator rate and clock rate on the CLKIN pin */
const uint32_t OscRateIn = 12000000;
const uint32_t ExtRateIn = 0;

namespace arm_pro_mini {

  void setup() {
    SystemCoreClockUpdate();
    Chip_GPIO_Init(LPC_GPIO);
  }

  // This data must be global because we access it after changing the stack
  // for the ROM entry point.
  static unsigned int command[5];
  static unsigned int result[4];

  #define init_msdstate() *((uint32_t *)(0x10000054)) = 0x0

  // This function resets some microcontroller peripherals to reset
  // hardware configuration to ensure that the USB In-System Programming module
  // will work properly. It is normally called from reset and assumes some reset
  // configuration settings for the MCU.
  // Some of the peripheral configurations may be redundant in your specific
  // project.
  //
  // TODO: this seems to works but require additional review.
  void ReinvokeISP()
  {
    // Disable SYSTICK timer and interrupt before calling into ISP */
    SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);

    // Disable USB interrupts.
    NVIC_DisableIRQ(USB0_IRQn);

    /* make sure USB clock is turned on before calling ISP */
    LPC_SYSCTL->SYSAHBCLKCTRL |= 0x04000;
    /* make sure 32-bit Timer 1 is turned on before calling ISP */
    LPC_SYSCTL->SYSAHBCLKCTRL |= 0x00400;
    /* make sure GPIO clock is turned on before calling ISP */
    LPC_SYSCTL->SYSAHBCLKCTRL |= 0x00040;
    /* make sure IO configuration clock is turned on before calling ISP */
    LPC_SYSCTL->SYSAHBCLKCTRL |= 0x10000;

    /* make sure AHB clock divider is 1:1 */
    LPC_SYSCTL->SYSAHBCLKDIV = 1;

    // TODO: test this with an actual active WDT. (do we need it at all?
    // the ROM functionality may disable the WDT anyway).
    Chip_WWDT_DeInit(LPC_WWDT);

    // Send Reinvoke ISP command to ISP entry point.
    command[0] = 57;

    // Initialize Storage state machine.
    // TODO: what does this mean? Do we need it?
    init_msdstate();

    // Set stack pointer to ROM value (reset default) This must be the last
    // piece of code executed before calling ISP, because most C expressions
    // and function returns will fail after the stack pointer is changed. */
    __set_MSP(*((uint32_t *)0x00000000));

    // Enter ISP. We call "iap_entry" to enter ISP because the ISP entry is done
    // through the same command interface as IAP. */
    // xiap_entry(command, result);
    iap_entry(command, result);
    // Not supposed to come back!
  }

} // namespace arm_pro_mini
