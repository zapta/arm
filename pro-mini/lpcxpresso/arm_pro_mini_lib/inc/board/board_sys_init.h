
#ifndef __BOARD_SYS_INIT_H_
#define __BOARD_SYS_INIT_H_

#include "chip/lpc_types.h"

// Called prior to the application and sets up system
// clocking, memory, and any resources needed prior to the application
// starting.
void Board_SystemInit(void);

#endif /* __BOARD_SYS_INIT_H_ */
