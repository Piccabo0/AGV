#ifndef __Delay_h
#define __Delay_h

#include "stm32f1xx.h"

extern uint16_t Timer_debug;
extern uint8_t debug_show;
extern uint32_t NumOfSysTickInt;


extern void SysTick_IrqHandler(void);


#endif


