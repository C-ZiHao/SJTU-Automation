
/**
 * @file    board.h
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include "headfile.h"


/**
 * @brief	The board name 
 */
#define BOARD_NAME "board"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


/* I2C */
#define I2C_EEPROM_CLK_SRC I2C0_CLK_SRC
#define I2C_EEPROM_CLK_FREQ CLOCK_GetFreq(I2C0_CLK_SRC)
#define I2C_EEPROM_BASEADDR I2C0
#define I2C_EEPROM_SLAVE_ADDR_7BIT 0x50U
#define I2C_EEPROM_BAUDRATE 100000U
#define I2C_EEPROM_DATA_LENGTH 64U

#define I2C_CAT9555_CLK_SRC I2C3_CLK_SRC
#define I2C_CAT9555_CLK_FREQ CLOCK_GetFreq(I2C3_CLK_SRC)
#define I2C_CAT9555_BASEADDR I2C3
#define I2C_CAT9555_SLAVE_ADDR_7BIT 0x22U
#define I2C_CAT9555_BAUDRATE 200000U
#define I2C_CAT9555_DATA_LENGTH 2U

/* GPIO operation shortcuts */
#define BEEP_ON() GPIO_PortSet(BOARD_INITPINS_BEEP_GPIO, BOARD_INITPINS_BEEP_GPIO_PIN_MASK)
#define BEEP_OFF() GPIO_PortClear(BOARD_INITPINS_BEEP_GPIO, BOARD_INITPINS_BEEP_GPIO_PIN_MASK)

#define QESA() GPIO_PinRead(BOARD_INITPINS_QESa_GPIO, BOARD_INITPINS_QESa_PIN)
#define QESB() GPIO_PinRead(BOARD_INITPINS_QESb_GPIO, BOARD_INITPINS_QESb_PIN)
#define QESP() GPIO_PinRead(BOARD_INITPINS_QESp_GPIO, BOARD_INITPINS_QESp_PIN)

#define SDCD() GPIO_PinRead(BOARD_INITPINS_SDCD_GPIO, BOARD_INITPINS_SDCD_PIN)
#define SDWP() GPIO_PinRead(BOARD_INITPINS_SDWP_GPIO, BOARD_INITPINS_SDWP_PIN)

// Rotary Encoder Switch IRQ
//#define BOARD_RESa_IRQ PORTB_IRQn
//#define BOARD_RESa_IRQ_HANDLER PORTB_IRQHandler


// Rotary Encoder Switch IRQ
#define BOARD_RESa_IRQ PORTB_IRQn
//#define BOARD_RESa_IRQ_HANDLER PORTB_IRQHandler


/**
 * @brief 	Initialize board specific settings.
 */

void BOARD_InitDebugConsole(void);

uint8_t InverseByteBits(uint8_t data);
void BOARD_InitI2C(void);
void BOARD_I2C_GPIO(uint16_t ctrlValue);
void BOARD_InitRTC(void);
void Board_InitAll(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */


