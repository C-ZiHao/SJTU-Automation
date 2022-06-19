#ifndef _SEG_H_
#define _SEG_H_

#include "headfile.h"

#define DSPdp_ON() GPIO_PortClear(BOARD_INITPINS_DSPdp_GPIO, BOARD_INITPINS_DSPdp_GPIO_PIN_MASK)
#define DSPdp_OFF() GPIO_PortSet(BOARD_INITPINS_DSPdp_GPIO, BOARD_INITPINS_DSPdp_GPIO_PIN_MASK)
#define DSPdp_TOGGLE() GPIO_PortToggle(BOARD_INITPINS_DSPdp_GPIO, BOARD_INITPINS_DSPdp_GPIO_PIN_MASK)

void ShowNumHEX(uint8_t num);
void ShowNumDEC(uint8_t num);
void ShowNumOFF(void);


#endif /* _SEG_H_ */

