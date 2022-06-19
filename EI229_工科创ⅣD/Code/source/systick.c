
/**
 * @file    sysytick.c
 * @brief   systick_delay
 * @data    2020/9/30
 */

#include <systick.h>
//-------------------------------------------------------------------------------------------------------------------
//  @brief      sysytick_delay assign time
//  @return     void
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void systick_delay(uint32_t time)
{
    if(time == 0)   return;
    assert(SysTick_LOAD_RELOAD_Msk >= time);//time<=SysTick_LOAD_RELOAD_Msk
    SysTick->CTRL = 0x00;
    SysTick->LOAD = time;
    SysTick->VAL = 0x00;
    SysTick->CTRL = ( 0 | SysTick_CTRL_ENABLE_Msk
                  //| SysTick_CTRL_TICKINT_Msk
                    | SysTick_CTRL_CLKSOURCE_Msk
                );
    while( !(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
}
//-------------------------------------------------------------------------------------------------------------------
//  @brief      sysytick_delay assign time_ms
//  @return     void
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void systick_delay_ms(uint32_t ms)
{
	while(ms--) systick_delay(SYSTICK_SOURCE_CLOCK/1000);
}
//-------------------------------------------------------------------------------------------------------------------
//  @brief      delay short time
//  @return     void
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------

void delay(void)
{
    volatile uint32_t i = 0;
    for (i = 0; i < 5000; ++i)
    {
        __asm("NOP");
    }
}


