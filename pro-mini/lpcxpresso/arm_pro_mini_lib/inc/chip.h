// NOTE: a simplified version of lpc_chip_11uxx_lib LPC library.

#ifndef __CHIP_H_
#define __CHIP_H_

#include "chip/lpc_types.h"
#include "chip/sys_config.h"
#include "chip/cmsis.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CORE_M0
#error CORE_M0 is not defined for the LPC11xx architecture
#error CORE_M0 should be defined as part of your compiler define list
#endif

#ifndef CHIP_LPC11UXX
#error "Unexpected CPU type"
#endif

// Peripheral declarations.
#define LPC_I2C_BASE              0x40000000
#define LPC_WWDT_BASE             0x40004000
#define LPC_USART_BASE            0x40008000
#define LPC_TIMER16_0_BASE        0x4000C000
#define LPC_TIMER16_1_BASE        0x40010000
#define LPC_TIMER32_0_BASE        0x40014000
#define LPC_TIMER32_1_BASE        0x40018000
#define LPC_ADC_BASE              0x4001C000
#define LPC_DAC_BASE              0x40024000
#define LPC_ACMP_BASE             0x40028000
#define LPC_PMU_BASE              0x40038000
#define LPC_FLASH_BASE            0x4003C000
#define LPC_SSP0_BASE             0x40040000
#define LPC_IOCON_BASE            0x40044000
#define LPC_SYSCTL_BASE           0x40048000
#define LPC_USB0_BASE             0x40080000
#define LPC_CAN0_BASE             0x40050000
#define LPC_SSP1_BASE             0x40058000
#define LPC_GPIO_PIN_INT_BASE     0x4004C000
#define LPC_GPIO_GROUP_INT0_BASE  0x4005C000
#define LPC_GPIO_GROUP_INT1_BASE  0x40060000
#define LPC_GPIO_PORT_BASE        0x50000000

// Used to call the ROM functions.
#define IAP_ENTRY_LOCATION        0X1FFF1FF1
#define LPC_ROM_API_BASE_LOC      0x1FFF1FF8


#define LPC_I2C                   ((LPC_I2C_T              *) LPC_I2C_BASE)
#define LPC_WWDT                  ((LPC_WWDT_T             *) LPC_WWDT_BASE)
#define LPC_USART                 ((LPC_USART_T            *) LPC_USART_BASE)
#define LPC_TIMER16_0             ((LPC_TIMER_T            *) LPC_TIMER16_0_BASE)
#define LPC_TIMER16_1             ((LPC_TIMER_T            *) LPC_TIMER16_1_BASE)
#define LPC_TIMER32_0             ((LPC_TIMER_T            *) LPC_TIMER32_0_BASE)
#define LPC_TIMER32_1             ((LPC_TIMER_T            *) LPC_TIMER32_1_BASE)
#define LPC_ADC                   ((LPC_ADC_T              *) LPC_ADC_BASE)
#define LPC_PMU                   ((LPC_PMU_T              *) LPC_PMU_BASE)
#define LPC_FMC                   ((LPC_FMC_T              *) LPC_FLASH_BASE)
#define LPC_SSP0                  ((LPC_SSP_T              *) LPC_SSP0_BASE)
#define LPC_IOCON                 ((LPC_IOCON_T            *) LPC_IOCON_BASE)
#define LPC_SYSCTL                ((LPC_SYSCTL_T           *) LPC_SYSCTL_BASE)
#define LPC_SSP1                  ((LPC_SSP_T              *) LPC_SSP1_BASE)
#define LPC_USB                   ((LPC_USB_T              *) LPC_USB0_BASE)
#define LPC_PININT                ((LPC_PIN_INT_T          *) LPC_GPIO_PIN_INT_BASE)
#define LPC_GPIOGROUP             ((LPC_GPIOGROUPINT_T     *) LPC_GPIO_GROUP_INT0_BASE)
#define LPC_GPIO                  ((LPC_GPIO_T             *) LPC_GPIO_PORT_BASE)
#define LPC_ROM_API               (*((LPC_ROM_API_T        * *) LPC_ROM_API_BASE_LOC))

/**
 * @brief	System oscillator rate
 * This value is defined externally to the chip layer and contains
 * the value in Hz for the external oscillator for the board. If using the
 * internal oscillator, this rate can be 0.
 */
extern const uint32_t OscRateIn;

/**
 * @brief	Clock rate on the CLKIN pin
 * This value is defined externally to the chip layer and contains
 * the value in Hz for the CLKIN pin for the board. If this pin isn't used,
 * this rate can be 0.
 */
extern const uint32_t ExtRateIn;

#include "chip/pmu_11xx.h"
#include "chip/fmc_11xx.h"
#include "chip/sysctl_11xx.h"
#include "chip/clock_11xx.h"
#include "chip/iocon_11xx.h"
#include "chip/timer_11xx.h"
#include "chip/uart_11xx.h"
#include "chip/wwdt_11xx.h"
#include "chip/ssp_11xx.h"
#include "chip/romapi_11xx.h"
#include "chip/adc_11xx.h"
#include "chip/gpio_11xx_1.h"
#include "chip/gpiogroup_11xx.h"
#include "chip/pinint_11xx.h"
#include "chip/i2c_11xx.h"
#include "chip/i2cm_11xx.h"
#include "chip/usbd_11xx.h"
//#endif

/* Family specific IRQ handler alias list */
#define UART_IRQHandler      USART_IRQHandler
#define USART0_IRQHandler    USART_IRQHandler

/* Common IRQ Handler Alias list */
#define UART0_IRQHanlder     UART_IRQHandler
#define I2C0_IRQHandler      I2C_IRQHandler
#define CMP_IRQHandler       ACMP_IRQHandler
#define WWDT_IRQHandler      WDT_IRQHandler

/**
 * @brief	Current system clock rate, mainly used for sysTick
 */
extern uint32_t SystemCoreClock;

/**
 * @brief	Update system core clock rate, should be called if the
 *			system has a clock rate change
 * @return	None
 */
void SystemCoreClockUpdate(void);

/**
 * @brief	Set up and initialize hardware prior to call to main()
 * @return	None
 * @note	Chip_SystemInit() is called prior to the application and sets up
 * system clocking prior to the application starting.
 */
void Chip_SystemInit(void);

#ifdef __cplusplus
}
#endif

#endif /* __CHIP_H_ */
