// Phone hook controller.

#ifndef HOOK_H
#define HOOK_H

#include "mbed.h"

namespace hook {

// Call once upon initialization.
extern void initialize();

// Call from main loop.
extern void loop();

extern bool is_hook_stable_on();
extern bool is_hook_stable_off();

extern void set_hook(bool is_on);

}  // hook

#endif  // HOOK_H
