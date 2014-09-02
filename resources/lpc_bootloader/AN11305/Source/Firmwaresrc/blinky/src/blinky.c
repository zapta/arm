/****************************************************************************
 *   $Id:: blinky.c 4101 2012-11-08 16:52:26Z usb00423                   $
 *   Project: NXP LPC11Uxx 32-bit blinky example
 *
 *   Description:
 *   This file contains LED blink code example which include timer,
 *   GPIO initialization, and clock monitoring.
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

* Permission to use, copy, modify, and distribute this software and its 
* documentation is hereby granted, under NXP Semiconductors' 
* relevant copyright in the software, without fee, provided that it 
* is used in conjunction with NXP Semiconductors microcontrollers.  This 
* copyright, permission, and disclaimer notice must appear in all copies of 
* this code.

****************************************************************************/

#include "LPC11Uxx.h"                        /* LPC11xx definitions */
#include "timer32.h"
#include "gpio.h"

#define TEST_TIMER_NUM		0		/* 0 or 1 for 32-bit timers only */

extern volatile uint32_t timer32_0_counter[4];
extern volatile uint32_t timer32_1_counter[4];

#define NO_CRP          0xFFFFFFFF

// Disables UART and USB In System Programming (reads and writes)
// Leaves SWD debugging, with reads and writes, enabled
#define NO_ISP_MAGIC    0x4E697370

// Disables SWD debugging & JTAG, leaves ISP with with reads and writes enabled
// You will need UART connectivity and FlashMagic (flashmagictool.com) to reverse
// this. Don't even try this without these tools; most likely the SWD flash
// programming will not even complete.
// Allows reads and writes only to RAM above 0x10000300 and flash other than
// sector 0 (the first 4 kB). Full erase also allowed- again only through UART
// and FlashMagic (NO JTAG/SWD)
#define CRP1_MAGIC      0x12345678

// Disables SWD debugging & JTAG, leaves UART ISP with with only full erase
// enabled. You must have UART access and FlashMagic before setting this
// option.
// Don't even try this without these tools; most likely the SWD flash
// programming will not even complete.
#define CRP2_MAGIC      0x87654321

/************************************************************/
/**** DANGER CRP3 WILL LOCK PART TO ALL READS and WRITES ****/
/*********** #define CRP3_MAGIC xxxx 0x43218765 *************/
/************************************************************/

// This value is placed in flash at 0x000002FC by the script LPC1343_Flash_CRP.icf or crp_enabled.ld
#define CURRENT_CRP_SETTING NO_CRP

#ifdef __GNUC__
__attribute__ ((section(".crp"))) const uint32_t CRP_WORD = CURRENT_CRP_SETTING;
#endif
#ifdef __IAR_SYSTEMS_ICC__
const __root uint32_t CRP_WORD @ ".crp" = CURRENT_CRP_SETTING;
#endif


/* Main Program */
int main (void) 
{
  
  SystemCoreClockUpdate();

  /* TEST_TIMER_NUM is either 0 or 1 for 32-bit timer 0 or 1. */
  init_timer32(TEST_TIMER_NUM, TIME_INTERVAL);
  enable_timer32(TEST_TIMER_NUM);

  /* Enable AHB clock to the GPIO domain. */
  LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);
    
  /* Set port 0_7 to output */
  GPIOSetDir( 0, 22, 1 );

  while (1)                                /* Loop forever */
  {
#if TEST_TIMER_NUM
	/* I/O configuration and LED setting pending. */
	if ( (timer32_1_counter[0] > 0) && (timer32_1_counter[0] <= 50) )
	{
	  GPIOSetBitValue( 0, 22, 0 );
	}
	if ( (timer32_1_counter[0] > 50) && (timer32_1_counter[0] <= 100) )
	{
	  GPIOSetBitValue( 0, 22, 1 );
	}
	else if ( timer32_1_counter[0] > 100 )
	{
	  timer32_1_counter[0] = 0;
	}
#else
	/* I/O configuration and LED setting pending. */
	if ( (timer32_0_counter[0] > 0) && (timer32_0_counter[0] <= 50) )
	{
	  GPIOSetBitValue( 0, 22, 0 );
	}
	if ( (timer32_0_counter[0] > 50) && (timer32_0_counter[0] <= 100) )
	{
	  GPIOSetBitValue( 0, 22, 1 );
	}
	else if ( timer32_0_counter[0] > 100 )
	{
	  timer32_0_counter[0] = 0;
	}
#endif
  }
}
