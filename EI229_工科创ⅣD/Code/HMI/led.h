

#ifndef _led_h
#define _led_h

#include "headfile.h"


#define LED1_ON() GPIO_PortClear(BOARD_INITPINS_LED1_GPIO, BOARD_INITPINS_LED1_GPIO_PIN_MASK)
#define LED1_OFF() GPIO_PortSet(BOARD_INITPINS_LED1_GPIO, BOARD_INITPINS_LED1_GPIO_PIN_MASK)
#define LED1_TOGGLE() GPIO_PortToggle(BOARD_INITPINS_LED1_GPIO, BOARD_INITPINS_LED1_GPIO_PIN_MASK)
#define LED2_ON() GPIO_PortClear(BOARD_INITPINS_LED2_GPIO, BOARD_INITPINS_LED2_GPIO_PIN_MASK)
#define LED2_OFF() GPIO_PortSet(BOARD_INITPINS_LED2_GPIO, BOARD_INITPINS_LED2_GPIO_PIN_MASK)
#define LED2_TOGGLE() GPIO_PortToggle(BOARD_INITPINS_LED2_GPIO, BOARD_INITPINS_LED2_GPIO_PIN_MASK)

void LED2_flash(uint32_t delay_time);
void LED1_flash(uint32_t delay_time);

#endif
