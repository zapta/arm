// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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

// An output pin.
class OutputPin {
public:
  OutputPin(uint8 port_number, uint8 pin_number) :
      pin_gpio_B_(&LPC_GPIO->B[port_number][pin_number]) {
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, port_number, pin_number);
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
  InputPin(uint8 port_number, uint8 pin_number) :
      pin_gpio_B_(&LPC_GPIO->B[port_number][pin_number]) {
    Chip_GPIO_SetPinDIRInput(LPC_GPIO, port_number, pin_number);
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
