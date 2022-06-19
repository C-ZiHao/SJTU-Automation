#include "pin_mux.h"
#include "fsl_gpio.h"
#include "fsl_common.h"
#include "fsl_clock.h"

#define OLED_SCL_High GPIO_PortSet(BOARD_INITPINS_OLED_SCL_GPIO, BOARD_INITPINS_OLED_SCL_GPIO_PIN_MASK)
#define OLED_SCL_Low GPIO_PortClear(BOARD_INITPINS_OLED_SCL_GPIO, BOARD_INITPINS_OLED_SCL_GPIO_PIN_MASK)
#define OLED_SDA_High GPIO_PortSet(BOARD_INITPINS_OLED_SDA_GPIO, BOARD_INITPINS_OLED_SDA_GPIO_PIN_MASK)
#define OLED_SDA_Low GPIO_PortClear(BOARD_INITPINS_OLED_SDA_GPIO, BOARD_INITPINS_OLED_SDA_GPIO_PIN_MASK)
#define OLED_RST_High GPIO_PortSet(BOARD_INITPINS_OLED_RST_GPIO, BOARD_INITPINS_OLED_RST_GPIO_PIN_MASK)
#define OLED_RST_Low GPIO_PortClear(BOARD_INITPINS_OLED_RST_GPIO, BOARD_INITPINS_OLED_RST_GPIO_PIN_MASK)
#define OLED_DC_High GPIO_PortSet(BOARD_INITPINS_OLED_DC_GPIO, BOARD_INITPINS_OLED_DC_GPIO_PIN_MASK)
#define OLED_DC_Low GPIO_PortClear(BOARD_INITPINS_OLED_DC_GPIO, BOARD_INITPINS_OLED_DC_GPIO_PIN_MASK)
#define ms2delay(x) (uint32_t)MSEC_TO_COUNT((uint16_t)x, CLOCK_GetFreq(SYS_CLK))

void OLED_Delay_ms(uint16_t ms);
void OLED_Init(void);
void OLED_Fill(uint8_t dat);
void OLED_WrDat(uint8_t data);
void OLED_Wr6Dat(uint8_t data);
void OLED_WrCmd(uint8_t cmd);
void OLED_PutPixel(uint8_t x, uint8_t y);
void OLED_ClrPixel(uint8_t x, uint8_t y);
void OLED_Set_Pos(uint8_t x, uint8_t y);
void OLED_P6x8Str(uint8_t x, uint8_t y, uint8_t ch[]);
void OLED_P8x16Str(uint8_t x, uint8_t y,uint8_t ch[]);
void OLED_P14x16Str(uint8_t x, uint8_t y, uint8_t ch[]);
void OLED_HEXACSII(uint16_t hex, uint8_t* Print);
void OLED_Print_Num(uint8_t x, uint8_t y, uint16_t num);
void OLED_Print_Num1(uint8_t x, uint8_t y, uint16_t num);
void OLED_Rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t gif);
void OLED_Show_BMP(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t * bmp);
void dis_bmp(uint16_t high, uint16_t width, uint8_t *p,uint8_t value);
void OLED_uint16(uint8_t x, uint8_t y, uint16_t num);
void OLED_int16(uint8_t x, uint8_t y, int16_t num);

