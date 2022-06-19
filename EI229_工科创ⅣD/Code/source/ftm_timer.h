

#ifndef _FTMTIMER_h
#define _FTMTIMER_h

#include "headfile.h"

/* The Flextimer instance/channel used for board */
#define BOARD_FTM_BASEADDR FTM3

/* Interrupt number and interrupt handler for the FTM instance used */
#define BOARD_FTM_IRQ_NUM FTM3_IRQn
#define BOARD_FTM_HANDLER FTM3_IRQHandler

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK (CLOCK_GetFreq(kCLOCK_BusClk) / 4)

void BOARD_FTM_HANDLER(void);

void ftm_init();

#endif
