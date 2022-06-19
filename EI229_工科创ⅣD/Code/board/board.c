/**
 * @file    board.c
 * @brief   Board initialization file.
 */
 
/* This is a template for board specific configuration created by MCUXpresso IDE Project Wizard.*/

#include <stdint.h>
#include "board.h"
#include "fsl_i2c.h"
#include "fsl_ftm.h"
#include "fsl_rtc.h"
#include "fsl_clock.h"
#include "fsl_device_registers.h"

uint8_t g_master_txBuff[I2C_CAT9555_DATA_LENGTH];
uint8_t g_master_rxBuff[I2C_CAT9555_DATA_LENGTH];
uint8_t g_EEPROM_txBuff[I2C_EEPROM_DATA_LENGTH];
uint8_t g_EEPROM_rxBuff[I2C_EEPROM_DATA_LENGTH];

i2c_master_transfer_t masterXfer;
i2c_master_transfer_t masterXfer0;	/* for CAT24C256 EEPROM */



/**
 * @brief Set up and initialize all required blocks and functions related to the board hardware.
 */
void BOARD_InitDebugConsole(void) {
	/* The user initialization should be placed here */
}



uint8_t InverseByteBits(uint8_t data)
{
	uint8_t tempChar;

	tempChar = data;
	tempChar=(tempChar<<4)|(tempChar>>4);
	tempChar=((tempChar<<2)&0xcc)|((tempChar>>2)&0x33);
	tempChar=((tempChar<<1)&0xaa)|((tempChar>>1)&0x55);

	return tempChar;
}

void BOARD_InitI2C(void) {
	i2c_master_config_t masterConfig;
    uint32_t sourceClock;
    uint8_t deviceAddress;

    /* CAT9555 */
	I2C_MasterGetDefaultConfig(&masterConfig);
	masterConfig.baudRate_Bps = I2C_CAT9555_BAUDRATE;
	sourceClock = I2C_CAT9555_CLK_FREQ;
	I2C_MasterInit(I2C_CAT9555_BASEADDR, &masterConfig, sourceClock);
	delay();

	// Polarity
    g_master_txBuff[0] = 0x0U;
    g_master_txBuff[1] = 0x0U;
    deviceAddress     = 0x04U;
    masterXfer.slaveAddress   = I2C_CAT9555_SLAVE_ADDR_7BIT;
    masterXfer.direction      = kI2C_Write;
    masterXfer.subaddress     = (uint32_t)deviceAddress;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = g_master_txBuff;
    masterXfer.dataSize       = I2C_CAT9555_DATA_LENGTH;
    masterXfer.flags          = kI2C_TransferDefaultFlag;
    I2C_MasterTransferBlocking(I2C_CAT9555_BASEADDR, &masterXfer);
    delay();

    // IO Configuration
    g_master_txBuff[0] = 0U;
    g_master_txBuff[1] = 0U;
    deviceAddress     = 0x06U;
    masterXfer.subaddress     = (uint32_t)deviceAddress;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = g_master_txBuff;
    masterXfer.dataSize       = I2C_CAT9555_DATA_LENGTH;
    I2C_MasterTransferBlocking(I2C_CAT9555_BASEADDR, &masterXfer);
    delay();
}

void BOARD_I2C_GPIO(uint16_t ctrlValue) {
    uint8_t deviceAddress;

    g_master_txBuff[0] = ~(ctrlValue&0xFFU);
    g_master_txBuff[1] = ~(InverseByteBits((ctrlValue>>8)&0xFFU));
    deviceAddress     = 0x02U;
    masterXfer.slaveAddress   = I2C_CAT9555_SLAVE_ADDR_7BIT;
    masterXfer.direction      = kI2C_Write;
    masterXfer.subaddress     = (uint32_t)deviceAddress;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = g_master_txBuff;
    masterXfer.dataSize       = I2C_CAT9555_DATA_LENGTH;
    masterXfer.flags          = kI2C_TransferDefaultFlag;
    I2C_MasterTransferBlocking(I2C_CAT9555_BASEADDR, &masterXfer);
}


void BOARD_InitRTC() {
    rtc_config_t rtcConfig;
    rtc_datetime_t date;

    RTC_GetDefaultConfig(&rtcConfig);
    RTC_Init(RTC, &rtcConfig);
#if !(defined(FSL_FEATURE_RTC_HAS_NO_CR_OSCE) && FSL_FEATURE_RTC_HAS_NO_CR_OSCE)
    /* Select RTC clock source */
    RTC_SetClockSource(RTC);
#endif /* FSL_FEATURE_RTC_HAS_NO_CR_OSCE */

    /* Set a start date time and start RTC */
/*
    date.year   = 2020U;
    date.month  = 10U;
    date.day    = 8U;
    date.hour   = 14U;
    date.minute = 17U;
    date.second = 0U;
    RTC_StopTimer(RTC);
    RTC_SetDatetime(RTC, &date);
    RTC_StartTimer(RTC);
*/
}


void Board_InitAll(void)
{
	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	BOARD_InitI2C();
	BOARD_InitRTC();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
	/* Init FSL debug console. */
	BOARD_InitDebugConsole();
#endif
}
