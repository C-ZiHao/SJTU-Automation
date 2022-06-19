
#ifndef _SYSTICK_H_
#define _SYSTICK_H_

#include "headfile.h"

#define SYSTICK_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_CoreSysClk)

void systick_delay(uint32_t time);

void systick_delay_ms(uint32_t ms);
#define systick_delay_us(time)      systick_delay(USEC_TO_COUNT(time, SYSTICK_SOURCE_CLOCK))
#define systick_delay_ns(time)      systick_delay(USEC_TO_COUNT(time, SYSTICK_SOURCE_CLOCK/1000))

void delay(void);

#endif
