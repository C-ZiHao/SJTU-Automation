
/**
 * @file    Seg.c
 * @brief   Seg Show Num
 * @data    2020/10/6
 */
#include <seg.h>
static uint8_t DSPTable[] = { 0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10, 0x08, 0x03, 0x27, 0x21, 0x06, 0x0E };

void ShowNumHEX(uint8_t num) {
	uint16_t l;
	l = DSPTable[num & 0x0F];

	// DSP: PD9~PD15
	GPIO_PortSet(BOARD_INITPINS_DSPa_GPIO, 0x7F << BOARD_INITPINS_DSPa_PIN);	// turn off all DSP segments
	GPIO_PortClear(BOARD_INITPINS_DSPa_GPIO, ((~l)&0x7F) << BOARD_INITPINS_DSPa_PIN);	// turn on the code segments
}

void ShowNumDEC(uint8_t num) {
	uint16_t l;
	l = DSPTable[num % 10];

	// DSP: PD9~PD15
	GPIO_PortSet(BOARD_INITPINS_DSPa_GPIO, 0x7F << BOARD_INITPINS_DSPa_PIN);	// turn off all DSP segments
	GPIO_PortClear(BOARD_INITPINS_DSPa_GPIO, ((~l)&0x7F) << BOARD_INITPINS_DSPa_PIN);	// turn on the code segments
}

void ShowNumOFF(void) {
	// DSP: PD9~PD15
	GPIO_PortSet(BOARD_INITPINS_DSPa_GPIO, 0x7F << BOARD_INITPINS_DSPa_PIN);	// turn off all DSP segments
}
