
/**
 * @file    MEM.c
 * @brief   MEM read
 * @data    2020/10/16
 */
#include <MEM.h>

volatile uint16_t MEM1_TEMP[8] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile uint16_t MEM2_TEMP[8] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile uint16_t MEM3_TEMP[8] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile uint16_t Light1_TEMP[8] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile uint16_t Light2_TEMP[8] = {0, 0, 0, 0, 0, 0, 0, 0};

MMA7260_t MEMS_temp;

void Light_get(PHOTODIODE_t *AnalogInput)
{
	 //Mean filtering
	 for (int i = 0; i <3; i++)
	 {
	 	  Light1_TEMP[i] = ADC_mid(Light1_BASE,Light1_CHANNEL);
	 	  Light2_TEMP[i] = ADC_mid(Light2_BASE,Light2_CHANNEL);
	 }
	 quicksort(Light1_TEMP, 4, 0, 3);
	 quicksort(Light2_TEMP, 4, 0, 3);
	 //the middle two
	 AnalogInput->L1 =(Light1_TEMP[1] + Light1_TEMP[2])/2;
	 AnalogInput->L2 =(Light2_TEMP[1] + Light2_TEMP[2])/2;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief         Get message from adc
//  @return        adc_value of MEMS
//  @data          2020/10/16
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
uint16_t ADC_convert(ADC_Type *adcn, uint16_t ch)
{
    adc16_channel_config_t adc16ChannelConfigStruct;

    adc16_channel_mux_mode_t ADCMUX;
    ADCMUX = kADC16_ChannelMuxA;
    ADC16_SetChannelMuxMode(ADC0, ADCMUX);
    ADC16_SetChannelMuxMode(ADC1, ADCMUX);

    adc16ChannelConfigStruct.channelNumber = ch;
    adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = false;

    ADC16_SetChannelConfig(adcn, 0, &adc16ChannelConfigStruct);
    while (0U == (kADC16_ChannelConversionDoneFlag &
                      ADC16_GetChannelStatusFlags(adcn, 0U))) {}
    uint16_t adc_value=(ADC16_GetChannelConversionValue(adcn, 0) & 0xFFF);
    return  adc_value;
}

uint16_t MEM_convertB(ADC_Type *adcn, uint16_t ch)
{
    adc16_channel_config_t adc16ChannelConfigStruct;

    adc16_channel_mux_mode_t ADCMUX;
    ADCMUX = kADC16_ChannelMuxB;
    ADC16_SetChannelMuxMode(ADC0, ADCMUX);
    ADC16_SetChannelMuxMode(ADC1, ADCMUX);

    adc16ChannelConfigStruct.channelNumber = ch;
    adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = false;

    ADC16_SetChannelConfig(adcn, 0, &adc16ChannelConfigStruct);
    while (0U == (kADC16_ChannelConversionDoneFlag &
                      ADC16_GetChannelStatusFlags(adcn, 0U))) {}
    uint16_t adc_value=(ADC16_GetChannelConversionValue(adcn, 0) & 0xFFF);
    return  adc_value;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief         Get adc value after fiter and normalizing
//  @return        Fianl adc_value of MEMS
//  @data          2020/10/16
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void MEMS_get(MMA7260_t *AnalogInput)
{
	 //Mean filtering
	 for (int i = 0; i <3; i++)
	 {
	 	  MEM1_TEMP[i] = ADC_mid(MEMSx_BASE,MEMSx_CHANNEL);
	 	  MEM2_TEMP[i] = ADC_mid(MEMSy_BASE,MEMSy_CHANNEL);
	 	  MEM3_TEMP[i] = ADC_mid(MEMSz_BASE,MEMSz_CHANNEL);
	 }
	 quicksort(MEM1_TEMP, 4, 0, 3);
	 quicksort(MEM2_TEMP, 4, 0, 3);
	 quicksort(MEM3_TEMP, 4, 0, 3);
	 //the middle two
	 AnalogInput->x =(MEM1_TEMP[1] + MEM1_TEMP[2])/2;
	 AnalogInput->y =(MEM2_TEMP[1] + MEM2_TEMP[2])/2;
	 AnalogInput->z =(MEM3_TEMP[1] + MEM3_TEMP[2])/2;

	 MEMS_Normalized(AnalogInput);

}

//-------------------------------------------------------------------------------------------------------------------
//  @brief         Normalized(-128 to 128)
//  @return        Normalize the sampled value
//  @data          2020/10/16
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void MEMS_Normalized(MMA7260_t *AnalogInput)
{
	//Low pass filter,removed finally
	/*
	AnalogInput->x=MEMS_temp.x*0.4+AnalogInput->x*0.6;
	AnalogInput->y=MEMS_temp.y*0.4+AnalogInput->y*0.6;
	AnalogInput->z=MEMS_temp.z*0.4+AnalogInput->z*0.6;
	MEMS_temp.x=AnalogInput->x;
	MEMS_temp.y=AnalogInput->y;
	MEMS_temp.z=AnalogInput->z;
	*/
	AnalogInput->x = (AnalogInput->x-MEMSx_MIN)*256/(MEMSx_MAX-MEMSx_MIN)-128;
	AnalogInput->y = (AnalogInput->y-MEMSy_MIN)*256/(MEMSy_MAX-MEMSy_MIN)-128;
    AnalogInput->z = (AnalogInput->z-MEMSz_MIN)*256/(MEMSz_MAX-MEMSz_MIN)-128;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief         mid filter
//  @return        Normalize the sampled value
//  @data          2020/10/16
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
uint16_t ADC_mid(ADC_Type *adcn, uint16_t ch)
{
    uint16_t tmp[3] = {0, 0, 0};
    for (int i = 0; i < 3; i++)
    {
    	tmp[i] = ADC_convert(adcn,ch);
    }
    quicksort(MEM1_TEMP, 3, 0, 2);

    return tmp[1];
}
//-------------------------------------------------------------------------------------------------------------------
//  @brief        Quick rooting
//  @data          2020/10/16
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
float Fsqrt(float x)
{
    float xhalf = 0.5f * x;
    int i = *(int *)&x; // evil floating point bit level hacking
    i = 0x5f375a86 - (i >> 1);
    x = *(float *)&i;
    x = x * (1.5f - (xhalf * x * x));
    return 1 / x;
}
//-------------------------------------------------------------------------------------------------------------------
//  @brief         swap
//  @data          2020/10/16
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void swap(volatile uint16_t *a, volatile uint16_t *b)
{
    int temp;

    temp = *a;
    *a = *b;
    *b = temp;

    return;
}
//-------------------------------------------------------------------------------------------------------------------
//  @brief         Quick sort
//  @data          2020/10/16
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void quicksort(volatile uint16_t array[], int maxlen, int begin, int end)
{
    int i, j;

    if (begin < end)
    {
        i = begin + 1; // 将array[begin]作为基准数，因此从array[begin+1]开始与基准数比较！
        j = end;       // array[end]是数组的最后一位

        while (i < j)
        {
            if (array[i] > array[begin]) // 如果比较的数组元素大于基准数，则交换位置。
            {
                swap(&array[i], &array[j]); // 交换两个数
                --j;
            }
            else
            {
                ++i; // 将数组向后移一位，继续与基准数比较。
            }
        }
        if (array[i] >= array[begin]) // 这里必须要取等“>=”，否则数组元素由相同的值时，会出现错误！
        {
            --i;
        }

        swap(&array[begin], &array[i]); // 交换array[i]与array[begin]

        quicksort(array, maxlen, begin, i);
        quicksort(array, maxlen, j, end);
    }
}
//-------------------------------------------------------------------------------------------------------------------
//  @brief         Origin MEMs_read
//  @data          2020/10/16
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void CDK66_MEMS(MMA7260_t *AnalogInput) {
    adc16_channel_config_t adc16ChannelConfigStruct;
    adc16_channel_mux_mode_t ADCMUX;

    ADCMUX = kADC16_ChannelMuxA;
    ADC16_SetChannelMuxMode(ADC0, ADCMUX);
    ADC16_SetChannelMuxMode(ADC1, ADCMUX);

    // MEMSx
    adc16ChannelConfigStruct.channelNumber                        = MEMSx_CHANNEL;
    adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = false;
    ADC16_SetChannelConfig(MEMSx_BASE, 0U, &adc16ChannelConfigStruct);
    while (0U == (kADC16_ChannelConversionDoneFlag &
                  ADC16_GetChannelStatusFlags(MEMSx_BASE, 0U))) {}
    AnalogInput->x = ADC16_GetChannelConversionValue(MEMSx_BASE, 0U) & 0xFFF;

    // MEMSy
    adc16ChannelConfigStruct.channelNumber                        = MEMSy_CHANNEL;
    adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = false;
    ADC16_SetChannelConfig(MEMSy_BASE, 0U, &adc16ChannelConfigStruct);
    while (0U == (kADC16_ChannelConversionDoneFlag &
                  ADC16_GetChannelStatusFlags(MEMSy_BASE, 0U))) {}
    AnalogInput->y = ADC16_GetChannelConversionValue(MEMSy_BASE, 0U) & 0xFFF;

    // MEMSz
    adc16ChannelConfigStruct.channelNumber                        = MEMSz_CHANNEL;
    adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = false;
    ADC16_SetChannelConfig(MEMSz_BASE, 0U, &adc16ChannelConfigStruct);
    while (0U == (kADC16_ChannelConversionDoneFlag &
                  ADC16_GetChannelStatusFlags(MEMSz_BASE, 0U))) {}
    AnalogInput->z = ADC16_GetChannelConversionValue(MEMSz_BASE, 0U) & 0xFFF;

    return;
}

