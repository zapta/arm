#ifndef IO_PINS_H
#define IO_PINS_H

// TODO: verify if Chip_GPIO_Init(LPC_GPIO) should be called upon
// initialization and if so, make sure it is called (maybe it already is
// called in the chip or board initialization).

#include "chip.h"

// Thin wrappers for working with individual digial I/O pins. Each pins
// addresses are cached on initialization to improve the speed. Unless
// specified otherwise, all methods are atomic (no need to protect with
// interrupt disable).
namespace io_pins {

namespace _private {
  // TODO: Fix a bug with some pin's function.

  // Return the io control pin mod function to select the pio function for
  // the given port/bit. This is an inconsistency where some bits required
  // FUNC0 while other required FUNC1. See 7.4.1 I/O configuration registers
  // section in LPC11U35 user's manual (document UM10462).
  inline uint32 pio_mod_function(uint8 port_index, uint8 bit_index) {
    return (port_index == 0 && bit_index >= 10 && bit_index <= 15)
        ? (IOCON_FUNC1 | IOCON_DIGMODE_EN) : (IOCON_FUNC0 | IOCON_DIGMODE_EN);
  }
}
// An output pin.
class OutputPin {
public:
  OutputPin(uint8 port_index, uint8 bit_index) :
      pin_gpio_B_(&LPC_GPIO->B[port_index][bit_index]) {
    // Select the PIO function for this pin.
    Chip_IOCON_PinMuxSet(LPC_IOCON, port_index, bit_index,
        _private::pio_mod_function(port_index, bit_index));
    // Set the pin as output.
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, port_index, bit_index);
    set(false);  // default state.
  }

  // Set the pin to given state (false = low, true = high).
  inline void set(bool state) {
    *pin_gpio_B_ = state;
  }

  // Return the pin's state.
  inline bool get() {
    return *pin_gpio_B_;
  }

private:
  // Cached pointer to byte register of this pin.
  volatile uint8* const pin_gpio_B_;
};

// An input pin.
class InputPin {
public:
  InputPin(uint8 port_index, uint8 bit_index) :
      pin_gpio_B_(&LPC_GPIO->B[port_index][bit_index]) {
    // Select the PIO function for this pin.
    Chip_IOCON_PinMuxSet(LPC_IOCON, port_index, bit_index,
        _private::pio_mod_function(port_index, bit_index));
    // Set the pin as output.
    Chip_GPIO_SetPinDIRInput(LPC_GPIO, port_index, bit_index);
  }

  // Return the pin's state.
  inline bool get() {
    return *pin_gpio_B_;
  }

private:
  // Cached pointer to byte register of this pin.
  volatile uint8* const pin_gpio_B_;
};

}  // namespace io_pins

#endif
