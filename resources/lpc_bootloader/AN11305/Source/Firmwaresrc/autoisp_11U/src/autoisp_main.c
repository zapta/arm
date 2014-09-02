/****************************************************************************
 *   $Id:: autoisp_main.c 4528 2010-08-24 23:32:22Z nxp21346                        $
 *   Project: NXP LPC13xx autoisp example
 *
 *   Description:
 *     This file contains the main routine for the autoisp code sample
 *     which puts an LPC134x part in USB ISP mode with the watchdog configured
 *     to reboot the chip after 15 seconds so it can run the new firmware.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/


#include "LPC11Uxx.h"
#include "gpio.h"
#include "clkconfig.h"
#include "config.h"
#include "wdt.h"

volatile uint32_t TimeTick = 0;

#define SYSTICK_TICKSPERSEC 100
#define SYSTICK_DELAY_CYCLES (SystemCoreClock/SYSTICK_TICKSPERSEC)
#define INTERNAL_RC_HZ 12000000

#define WDTCLK_SRC_IRC_OSC          0
#define WDTCLK_SRC_WDT_OSC          1


/* SysTick interrupt happens every 10 ms */
/* Note- The SysTick timer is in the Cortex CPU core and does not
 * run during sleep mode.
 */
void SysTick_Handler(void)
{
  /* Count 10ms periods */
  TimeTick++;
}


/* This data must be global so it is not read from the stack */
typedef void (*IAP)(uint32_t [], uint32_t []);
IAP iap_entry = (IAP)0x1fff1ff1;
uint32_t command[5], result[4];
#define init_msdstate() *((uint32_t *)(0x10000054)) = 0x0

/* This function resets some microcontroller peripherals to reset
   hardware configuration to ensure that the USB In-System Programming module
   will work properly. It is normally called from reset and assumes some reset
   configuration settings for the MCU.
   Some of the peripheral configurations may be redundant in your specific
   project.
*/
void ReinvokeISP(void)
{
  /* make sure USB clock is turned on before calling ISP */
  LPC_SYSCON->SYSAHBCLKCTRL |= 0x04000;
  /* make sure 32-bit Timer 1 is turned on before calling ISP */
  LPC_SYSCON->SYSAHBCLKCTRL |= 0x00400;
  /* make sure GPIO clock is turned on before calling ISP */
  LPC_SYSCON->SYSAHBCLKCTRL |= 0x00040;
  /* make sure IO configuration clock is turned on before calling ISP */
  LPC_SYSCON->SYSAHBCLKCTRL |= 0x10000;

  /* make sure AHB clock divider is 1:1 */
  LPC_SYSCON->SYSAHBCLKDIV = 1;

  /* Send Reinvoke ISP command to ISP entry point*/
  command[0] = 57;

  init_msdstate();					 /* Initialize Storage state machine */
  /* Set stack pointer to ROM value (reset default) This must be the last
     piece of code executed before calling ISP, because most C expressions
     and function returns will fail after the stack pointer is changed. */
  __set_MSP(*((uint32_t *)0x00000000));

  /* Enter ISP. We call "iap_entry" to enter ISP because the ISP entry is done
     through the same command interface as IAP. */
  iap_entry(command, result);
  // Not supposed to come back!
}

/* Main Program */
int main()
{
  /* Basic chip initialization is taken care of in SystemInit() called
   * from the startup code. Chip settings are defined in the CMSIS
   * system_<part family>.c file.
   */
  WDT_CLK_Setup(WDTCLK_SRC_WDT_OSC);

  /* Initialize GPIO (sets up clock) */
  GPIOInit();

  /* Configure SYSTICK timer to generate interrupt every 10ms */
  SysTick_Config(SYSTICK_DELAY_CYCLES);

  GPIOSetDir( LED_PORT, LED_BIT, 1);
  /*Turn on LED to indicate the user application is active*/
  GPIOSetBitValue( LED_PORT, LED_BIT, LED_ON);

  while(1)
  {

	GPIOSetBitValue( LED_PORT, LED_BIT, (TimeTick/(SYSTICK_TICKSPERSEC/2))&1);
    if(TimeTick >= PRE_ISP_DELAY_SECS*SYSTICK_TICKSPERSEC)
    {

      /* Disable SYSTICK timer and interrupt before calling into ISP */
      SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);

      /*Initialize WDT*/
      WDTInit();

      /* Turn off LED to indicate ISP is active */
      GPIOSetBitValue( LED_PORT, LED_BIT, LED_OFF);

      /* Start ISP */
      ReinvokeISP();
    }
    /* Go to sleep to save power between SYSTICK interrupts */
    __WFI();
  }
}
