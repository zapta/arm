/*
 Channel 	 	pin	fun	ADM	flt	Notes
 ADC_CH0		11	2	7.0	8.1
 ADC_CH1		12	2	7.0	8.1
 ADC_CH2		13	2	7.0	8.1
 ADC_CH3		14	2	7.0	8.1
 ADC_CH4		15	2	7.0	8.1 Pulled up with 10K res for SWD
 ADC_CH5		16	1	7.0	8.1
 ADC_CH6		22	1	7.0	8.1
 ADC_CH7		23	1	7.0	8.1
 */

#ifndef ADC_H
#define ADC_H

#include "chip.h"
#include "chip/adc_11xx.h"

namespace ADC {

bool ADC_initialized = false;

class ADC_Init {
public:
	ADC_Init(uint8 Channel) :
			Channel_(Channel) {
		// Set the PIO function for this pin to ADC.
		uint8 bit_index = (Channel_ <= 5) ? (Channel_ + 11) : (Channel_ + 16);
		uint8 modefunc = (Channel_ <= 4) ? (IOCON_FUNC2) : (IOCON_FUNC1);
		Chip_IOCON_PinMuxSet(LPC_IOCON, 0, bit_index,
				(modefunc | IOCON_ADMODE_EN | IOCON_FILT_DIS));
		//Initialize the ADC peripheral
		if (!ADC_initialized) {
			ADC_CLOCK_SETUP_T adc_clk;
			Chip_ADC_Init(LPC_ADC, &adc_clk);
			ADC_initialized = true;
		}

	}

	// Reads the value of this channel.
	inline uint16 Read() {
		//Enables the channel for ADC.
		Chip_ADC_EnableChannel(LPC_ADC, (ADC_CHANNEL_T) Channel_, ENABLE);
		// Buffer
		uint16 adc_result = 0;
		// Setting ADC to starting sampling now
		Chip_ADC_SetStartMode(LPC_ADC, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
		// Get the sampled data from ADC Channel Data Register
		Chip_ADC_ReadValue(LPC_ADC, (uint8_t) Channel_, &adc_result);
		//DISABLE the channel for ADC.
		//In software-controlled mode (BURST = 0), only one channel can be selected.
		//See 19.5.1 A/D Control Register section in LPC11U35 user's manual (document UM10462).
		Chip_ADC_EnableChannel(LPC_ADC, (ADC_CHANNEL_T) Channel_, DISABLE);
		return adc_result;
	}

private:
	// Cached buffer for this channel.
	volatile uint8 const Channel_;
};

}

#endif  // ADC_H
