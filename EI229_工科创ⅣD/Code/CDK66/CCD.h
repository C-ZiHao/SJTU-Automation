
#ifndef _CCD_H_
#define _CCD_H_

#include "headfile.h"
#include "MK66F18.h"

#define GrayScale (256)
#define USB_RING_BUFFER_SIZE	(128U)
#define cmdDataLenMax			(100U)

#define CCD_BASE ADC0
#define CCD_CHANNEL 14U

#define CCD_CLK_HIGH	GPIO_PinWrite(BOARD_INITPINS_CCDCLK_GPIO, BOARD_INITPINS_CCDCLK_PIN, 1)
#define CCD_CLK_LOW		GPIO_PinWrite(BOARD_INITPINS_CCDCLK_GPIO, BOARD_INITPINS_CCDCLK_PIN, 0)
#define CCD_SI_HIGH		GPIO_PinWrite(BOARD_INITPINS_CCDSI_GPIO, BOARD_INITPINS_CCDSI_PIN, 1)
#define CCD_SI_LOW		GPIO_PinWrite(BOARD_INITPINS_CCDSI_GPIO, BOARD_INITPINS_CCDSI_PIN, 0)


typedef struct _Board_Analog
 {
	 uint32_t	x;
	 uint32_t	y;
	 uint32_t	z;
	 uint32_t	CCD;
 } Board_Analog_t;


void line_find(void);
void Distortion_correction(void);

 int autoThreshold();
void Draw_LinearView(void);

void CCD_delay(void);
void LinearCameraOneShot(void);
void LinearCameraFlush(void);
void CollectCCD(void);

#endif
