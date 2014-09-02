/****************************************************************************
 *   $Id:: wdt.c 5316 2010-10-20 22:29:46Z usb01267                         $
 *   Project: auto ISP example
 *
 *   Description:
 *     This file contains WDT code example which include WDT 
 *     initialization, WDT interrupt handler, and APIs for WDT
 *     reading.
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

#include "LPC11Uxx.h"
#include "wdt.h"

volatile uint32_t wdt_counter;

/*****************************************************************************
** Function name:		WDTHandler
**
** Descriptions:		Watchdog timer interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void WDT_IRQHandler(void)
{
}

/*****************************************************************************
** Function name:		WDTInit
**
** Descriptions:		Initialize watchdog timer, install the
**						watchdog timer interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void WDTInit( void )
{
  uint32_t i, regVal;
  wdt_counter = 0;

  LPC_WWDT->TC = 0x7FFF;

  regVal = WDEN | WDRESET;
  LPC_WWDT->MOD = regVal;

  LPC_WWDT->FEED = 0xAA;		/* Feeding sequence */
  LPC_WWDT->FEED = 0x55;
  /* Make sure feed sequence executed properly */
  for (i = 0; i < 0x80; i++);

  return;
}

/*****************************************************************************
** Function name:		WDTFeed
**
** Descriptions:		Feed watchdog timer to prevent it from timeout
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void WDTFeed( void )
{
  LPC_WWDT->FEED = 0xAA;		/* Feeding sequence */
  LPC_WWDT->FEED = 0x55;
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
