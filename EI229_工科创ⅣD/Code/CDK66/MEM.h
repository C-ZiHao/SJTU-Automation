
#ifndef _MEM_H_
#define _MEM_H_

#include "headfile.h"

#define MEMSx_BASE ADC0
#define MEMSx_CHANNEL 19U
#define MEMSy_BASE ADC0
#define MEMSy_CHANNEL 3U
#define MEMSz_BASE ADC1
#define MEMSz_CHANNEL 19U

#define Light1_BASE ADC1
#define Light1_CHANNEL 18U
#define Light2_BASE ADC0
#define Light2_CHANNEL 23U

#define MEMSx_MAX 3220
#define MEMSx_MIN 1140
#define MEMSy_MAX 3460
#define MEMSy_MIN 1430
#define MEMSz_MAX 3240
#define MEMSz_MIN 1280



typedef struct _MMA7260
 {
	 int32_t	x;
	 int32_t	y;
	 int32_t	z;
 } MMA7260_t;

 typedef struct _PHOTODIODE
  {
 	 int32_t	L1;
 	 int32_t	L2;
 	 int32_t    error;
  } PHOTODIODE_t;

 void CDK66_MEMS(MMA7260_t *AnalogInput);
 void Light_get(PHOTODIODE_t *AnalogInput);
void MEMS_get(MMA7260_t *AnalogInput);
uint16_t ADC_mid(ADC_Type *adcn, uint16_t ch);
void MEMS_mean(MMA7260_t *AnalogInput);
uint16_t ADC_convert(ADC_Type *adcn, uint16_t ch);
uint16_t MEM_convertB(ADC_Type *adcn, uint16_t ch);
void MEMS_Normalized(MMA7260_t *AnalogInput);

float Fsqrt(float x);
void swap(volatile uint16_t *a, volatile uint16_t *b);
void quicksort(volatile uint16_t array[], int maxlen, int begin, int end);

#endif
