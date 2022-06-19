
/**
 * @file    led.c
 * @data    2020/9/29
 */

#include "led.h"
//-------------------------------------------------------------------------------------------------------------------
//  @brief      LED Flash
//  @return     void
//  @data       2020/9/29
//-------------------------------------------------------------------------------------------------------------------
void LED1_flash(uint32_t delay_time)
{
	 LED1_TOGGLE();
	 systick_delay_ms(delay_time);
}

void LED2_flash(uint32_t delay_time)
{
	 LED2_TOGGLE();
	 systick_delay_ms(delay_time);
}

