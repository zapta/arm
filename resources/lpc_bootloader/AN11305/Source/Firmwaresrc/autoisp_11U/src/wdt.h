/****************************************************************************
 *   $Id:: wdt.h 4110 2010-08-02 22:28:35Z usb01267                         $
 *   Project: NXP LPC11Uxx WDT example
 *
 *   Description:
 *     This file contains WDT code header definition.
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
#ifndef __WDT_H 
#define __WDT_H

#define WDEN              (0x1<<0)
#define WDRESET           (0x1<<1)
#define WDTOF             (0x1<<2)
#define WDINT             (0x1<<3)
#define WDPROTECT         (0x1<<4)
#define WDLOCKCLK         (0x1<<5)

#define WDT_FEED_VALUE		0x003FFFFF

#define WINDOW_MODE       0
#define PROTECT_MODE      1
#define WATCHDOG_RESET    0
#define WDLOCK_MODE       0
#define LOCKCLK_MODE      0

extern void WDT_IRQHandler(void);
extern void WDTInit( void );
extern void WDTFeed( void );

#endif /* end __WDT_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
