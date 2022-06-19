#ifndef _PITTIMER_H_
#define _PITTIMER_H_

#include "headfile.h"
#include "MK66F18.h"

// PIT0 provides app ticks (uint32) at an interval of 5ms
#define APPTICK_PIT_HANDLER PIT0_IRQHandler
#define APPTICK_PIT_IRQn PIT0_IRQn
#define APPTICK_PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)

#define Key_PIT_HANDLER PIT1_IRQHandler
#define Key_PIT_IRQn PIT1_IRQn
#define Key_PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)

#define ADC_PIT_HANDLER PIT2_IRQHandler
#define ADC_PIT_IRQn PIT2_IRQn
#define ADC_PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)



void InitPITs(void);
void APPTICK_PIT_HANDLER(void);

void Key_PIT_HANDLER(void);

#endif
