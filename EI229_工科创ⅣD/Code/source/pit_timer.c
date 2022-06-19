
/**
 * @file    pit_timer.c
 * @brief   pit timer
 * @data    2020/10/6
 */

#include "headfile.h"

volatile uint32_t appTick = 0U;		// tick at an interval of 1ms
extern KEY_MSG_t key1_msg,key2_msg;

void InitPITs() {
    pit_config_t pitConfig;
    PIT_GetDefaultConfig(&pitConfig);
    PIT_Init(PIT, &pitConfig);
    /* Set timer period for channel 0: 1000us = 1ms */
    PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, USEC_TO_COUNT(1000U, APPTICK_PIT_SOURCE_CLOCK));
    /* Enable timer interrupts for channel 0 */
    PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
    /* Enable at the NVIC */
    EnableIRQ(APPTICK_PIT_IRQn);
    /* Start channel 0 */
    PIT_StartTimer(PIT, kPIT_Chnl_0);

    /* Set timer period for channel 1: 10000us = 10ms */
    PIT_SetTimerPeriod(PIT, kPIT_Chnl_1, USEC_TO_COUNT(10000U, Key_PIT_SOURCE_CLOCK));
    /* Enable timer interrupts for channel 1 */
    PIT_EnableInterrupts(PIT, kPIT_Chnl_1, kPIT_TimerInterruptEnable);
    /* Enable at the NVIC */
    EnableIRQ(Key_PIT_IRQn);
    /* Start channel 0 */
    PIT_StartTimer(PIT, kPIT_Chnl_1);

    /* Set timer period for channel 2: 20000us = 20ms */
    PIT_SetTimerPeriod(PIT, kPIT_Chnl_2, USEC_TO_COUNT(150000U, ADC_PIT_SOURCE_CLOCK));
    /* Enable timer interrupts for channel 2 */
    PIT_EnableInterrupts(PIT, kPIT_Chnl_2, kPIT_TimerInterruptEnable);
    /* Enable at the NVIC */
    EnableIRQ(ADC_PIT_IRQn);
    /* Start channel 0 */
    PIT_StartTimer(PIT, kPIT_Chnl_2);

    appTick = 0;
}

extern MMA7260_t MEMS;
extern PHOTODIODE_t Light;
extern float CCD_error;
extern uint16_t threshold;
//-------------------------------------------------------------------------------------------------------------------
//  @brief      appTicks increasing at an interval of 10ms
//-------------------------------------------------------------------------------------------------------------------
void ADC_PIT_HANDLER(void)
{
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_2, kPIT_TimerFlag);

    /*
    LinearCameraOneShot();
    Distortion_correction();
    threshold=autoThreshold();
    threshold=MINMAX(60,threshold,120);
    line_find();
    CCD_cal();
    servo_control(CCD_error);
  */
    //Light_get(&Light);
    //cal_dir();
    //servo_control(Light.error);

#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      appTicks increasing at an interval of 1ms
//-------------------------------------------------------------------------------------------------------------------
void APPTICK_PIT_HANDLER(void)
{
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
    appTick++;
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}


//-------------------------------------------------------------------------------------------------------------------
//  @brief      Check key status regularly(10ms), using FIFO 1
//  @data       2020/10/6
//-------------------------------------------------------------------------------------------------------------------

extern KEY_MSG_t keymsg;
extern uint16_t mode, premode;
extern bool loop_flag;
void Key_PIT_HANDLER(void)
{
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_1, kPIT_TimerFlag);

    KEY_e   keynum;
    static uint8_t keytime[KEY_MAX];
    for(keynum = (KEY_e)0 ; keynum < KEY_MAX; keynum ++)
    {
        if(key_get(keynum) == KEY_DOWN)
        {
            keytime[keynum]++;
            if(keytime[keynum] == KEY_DOWN_DELAY_TIME + 1 )  //down
            {
                keymsg.key = keynum;
                keymsg.status[keynum] = KEY_DOWN;
                keymsg.count[keynum]++;
                send_key_msg(keymsg);
            }
            else if(keytime[keynum] > KEY_LONG_HOLD_TIME) //long press
            {
            	keymsg.key = keynum;
            	keymsg.status[keynum] = KEY_LONG_HOLD;
            	send_key_msg(keymsg);
            }
            else if(keytime[keynum] > KEY_HOLD_TIME)  //press
            {
                keymsg.key = keynum;
                keymsg.status[keynum] = KEY_HOLD;
                send_key_msg(keymsg);
            }
        }
        else
        {
            if(keytime[keynum] > KEY_DOWN_DELAY_TIME)  //up
            {
                keymsg.key = keynum;
                keymsg.status[keynum] = KEY_UP;
                send_key_msg(keymsg);
            }

            keytime[keynum] = 0;
        }
    }


#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif

}
