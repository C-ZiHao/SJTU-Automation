
/**
 * @file    ccd.c
 * @brief   ccd read
 * @data    2020/10/6
 */
#include <ccd.h>

extern uint16_t CCDData[128];
extern uint16_t NormalCCDData[128];
extern uint16_t threshold;
extern int32_t CCD_position,last_CCD_position;
extern float CCD_error;
uint16_t CCDBinary[128],CCDScore[128];
extern double direction;

void CCD_cal(void)
{
    CCD_error=-(CCD_position-64)*140/64* 0.3 + CCD_error*0.7;
	direction = -1.4*(CCD_position-64)/64* 0.3+direction*0.7;
    direction= MIN(MAX(direction, -0.99), 0.99);
}

void line_find(void)
{
	for(uint32_t i=0; i<128; i++)
	{
	     if(NormalCCDData[i]>threshold) CCDBinary[i]=255;
	     else CCDBinary[i]=0;
	}
	for(uint32_t i= 3; i<125; i++)
	{
	   if(CCDBinary[i-1]==0 && CCDBinary[i]==0 && CCDBinary[i+1]==0)  {CCDScore[i]=1;}
	   else  { CCDScore[i]=0;}
	}
	uint32_t max_score=0;
	for(int32_t i=3; i<125; i++)
	{
	   if(i<24)   { CCDScore[i]=i*CCDScore[i];}
	   else if(i>104)  {CCDScore[i]=(128-i)*CCDScore[i];}
	   else            {CCDScore[i]=25*CCDScore[i];}

	   if(abs(i-last_CCD_position)<20)
	   {
	        CCDScore[i]=(abs(i-last_CCD_position)+50)*CCDScore[i]/50;
	   }

	   if(max_score<CCDScore[i])
	   {
	       max_score=CCDScore[i];
	       CCD_position=i;
	   }
	}
	last_CCD_position=CCD_position;
}
void Distortion_correction(void)
{
	int32_t tempInt=0;
	for(tempInt=0; tempInt<128; tempInt++)
	{

	   if(tempInt%2==0)
	   {
	         CCDData[tempInt] = MAX(0,CCDData[tempInt]-96);	// Upper byte
	   }
	   else
	   {
	         CCDData[tempInt] = MAX(0,CCDData[tempInt]-83);	// Lower byte
	   }
	   if(tempInt<21)
	   {
	         int32_t tep=125*(20-tempInt); //118.75
	    	 CCDData[tempInt] =  MIN(CCDData[tempInt]+tep,4009);	// Upper byte
	   }
       else if(tempInt>128-20)
	   {
	         int32_t tep=250/2*(tempInt+20-128);
	    	 CCDData[tempInt] = MIN(CCDData[tempInt]+tep,4009);	// Upper byte
	    }
	}
	for(tempInt=0; tempInt<128; tempInt++)
	{
	      uint16_t data=MIN(255,CCDData[tempInt]*255/4009);
	      NormalCCDData[tempInt]=data;
	}

}

//============常规大津法============L
int autoThreshold()
{
  int pixelCount[GrayScale] = {0}; //每个灰度值所占像素个数
  float pixelPro[GrayScale] = {0}; //每个灰度值所占总像素比例
  int tmp_threshold = 0;

  //统计灰度级中每个像素在整幅图像中的个数
  for (int i = 0; i < 128; i++)
  {
      pixelCount[NormalCCDData[i]]++;
  }

  for(int i = 0; i < GrayScale; i++)
  {
     pixelPro[i] = (float)pixelCount[i] / 128; //计算每个像素在整幅图像中的比例
  }

  float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
  for (int i = 0; i < GrayScale; i++) // i作为阈值
  {
    w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
    for (int j = 0; j < GrayScale; j++)
    {
      if (j <= i) //背景部分
      {
        w0 += pixelPro[j];                             //背景像素点占整个图像的比例
        u0tmp += j * pixelPro[j];
      }
      else //前景部分
      {
        w1 += pixelPro[j];                             //前景像素点占整个图像的比例
        u1tmp += j * pixelPro[j];
      }
    }
    u0 = u0tmp / w0;                                 //背景平均灰度μ0
    u1 = u1tmp / w1;                                 //前景平均灰度μ1
    deltaTmp = (float)(w0 * w1 * pow((u0 - u1), 2)); //类间方差公式 g = w1 * w2 * (u1 - u2) ^ 2
    if (deltaTmp > deltaMax)
    {
      deltaMax = deltaTmp;
      tmp_threshold = i;
    }
  }
  return tmp_threshold;
}

void Draw_LinearView(void)
{
    uint8_t index=0, tempData;
    uint8_t i=0, j=0, k=0;

    for(index=0; index<128; index++)
	{
    	i = 7 - (CCDData[index]>>6)/8;
    	j = 7- (CCDData[index]>>6)%8;
    	for (k=0; k<8; k++)
    	{
    		OLED_Set_Pos(index, k);
    		if (i==k)
    		{
    			tempData = (1U << j);
    		}
    		else
    		{
    			tempData = 0;
    		}
    		OLED_WrDat(tempData);
    	}
	}
}


// simply generate SI & CLK pulses to flush the previously integrated frame
// while integrate new frame to be read out
void LinearCameraFlush(void)
{
    uint8_t index=0;

    CCD_SI_HIGH;
    CCD_delay();
    CCD_CLK_HIGH;
    CCD_delay();
    CCD_SI_LOW;
    CCD_delay();
    CCD_CLK_LOW;

    for(index=0; index<128; index++)
    {
        CCD_CLK_HIGH;
        CCD_delay();
        CCD_CLK_LOW;
        CCD_delay();
    }
}

void LinearCameraOneShot(void)
{
    uint8_t index=0;

    // flush previously integrated frame before capturing new frame
    LinearCameraFlush();
    // wait for TSL1401 to integrate new frame, exposure time control by delay
    systick_delay_ms(20);

    CCD_SI_HIGH;
    CCD_delay();
    CCD_CLK_HIGH;
    CCD_delay();
    CCD_SI_LOW;
    CCD_delay();
    CCD_CLK_LOW;

    // ADC mux to CCD channel
    ADC16_SetChannelMuxMode(ADC0, kADC16_ChannelMuxA);

    for(index=0; index<128; index++)
    {
    	// read AO value using ADC
        ADC16_SetChannelConfig(ADC0_PERIPHERAL, ADC0_CH2_CONTROL_GROUP, &ADC0_channelsConfig[2]);
        while (0U == (kADC16_ChannelConversionDoneFlag &
                      ADC16_GetChannelStatusFlags(ADC0_PERIPHERAL, ADC0_CH2_CONTROL_GROUP))) {}
        CCDData[index] = ADC16_GetChannelConversionValue(ADC0_PERIPHERAL, ADC0_CH2_CONTROL_GROUP) & 0xFFF;
        // clock pulse to read next pixel output
        CCD_CLK_HIGH;
        CCD_delay();
        CCD_CLK_LOW;
    }

    // 129th pulse to terminate output of 128th pixel
    CCD_CLK_HIGH;
    CCD_delay();
    CCD_CLK_LOW;
}

void CCD_delay()
{
	uint16_t delayCount;
	delayCount = 2;
	while(delayCount--) {}
}


/* Drive to and read from TSL1401CL */
void CollectCCD(void)
{
    uint8_t index=0;

    CCD_CLK_HIGH;
    CCD_delay();
    CCD_SI_LOW;
    CCD_delay();

    CCD_SI_HIGH;
    CCD_delay();
    CCD_CLK_LOW;
    CCD_delay();

    CCD_CLK_HIGH;
    CCD_delay();
    CCD_SI_LOW;
    CCD_delay();

    for(index=0; index<128; index++)
    {
        CCD_CLK_LOW;
        CCD_delay();

        ADC16_SetChannelConfig(ADC0_PERIPHERAL, ADC0_CH2_CONTROL_GROUP, &ADC0_channelsConfig[2]);
        while (0U == (kADC16_ChannelConversionDoneFlag &
                      ADC16_GetChannelStatusFlags(ADC0_PERIPHERAL, ADC0_CH2_CONTROL_GROUP))) {}
        CCDData[index] = ADC16_GetChannelConversionValue(ADC0_PERIPHERAL, ADC0_CH2_CONTROL_GROUP) & 0xFFF;

        CCD_CLK_HIGH;
        CCD_delay();
    }
}

