
/**
 * @file    ftm_timer.c
 * @brief   ftm timer,use pit finally
 * @data    2020/9/29
 */

#include <ftm_timer.h>
void ftm_init(void)
{
	ftm_config_t ftmInfo;
	FTM_GetDefaultConfig(&ftmInfo);
     /* Divide FTM clock by 4 */
	ftmInfo.prescale = kFTM_Prescale_Divide_4;
     /* Initialize FTM module */
	FTM_Init(BOARD_FTM_BASEADDR, &ftmInfo);

	FTM_SetTimerPeriod(BOARD_FTM_BASEADDR, USEC_TO_COUNT(1000U, FTM_SOURCE_CLOCK));
    FTM_EnableInterrupts(BOARD_FTM_BASEADDR, kFTM_TimeOverflowInterruptEnable);
	EnableIRQ(BOARD_FTM_IRQ_NUM);
    FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);
}

void BOARD_FTM_HANDLER(void)
{
    /* Clear interrupt flag.*/
    FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, kFTM_TimeOverflowFlag);
    __DSB();
}
