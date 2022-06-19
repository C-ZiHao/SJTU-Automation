
/**
 * @file      CDK66_Ref1.c
 * @brief     Lab4 of Engineering practice and technological innovation.
 *            mission : Familiar with the use of CCD
 *                      Use the data collected by CCD to control the status of the servo
 * @Features  a. Complete the basic library, conducive to follow-up experiments
 *            b. Use serial port to debug and visualize on the host computer
 *            c. Use filtering and distortion correction algorithm to ensure the accuracy of CCD data
 *            d. Use pid algorithm to quickly reach the designated position
 *            e. Use difference ratio and algorithm to ensure the accuracy of control
 *            f. Put the main control function into the interrupt for precise execution
 *            g. Familiar with ADC sampling, complete hardware configuration
 *
 * @TeamNum   G7
 * @Team      @Chai_Zihao @Xu_Yanxuan @Chen_Dengjian @Lai_Wentao
 * @data      2020/10/25
 */
#include "headfile.h"
#include "math.h"

#define MINMAX(x, l, u) MIN(MAX(x, l), u)

#define BYTE0(dwTemp) (*(char *)(&dwTemp))
#define BYTE1(dwTemp) (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp) (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp) (*((char *)(&dwTemp) + 3))

void Init_All(void);
void PcCmdProcess(void);
void DCAM_IRQHANDLER(void);

//KEY
#define QES_MIDDLE_VAR 128
volatile uint16_t QESVar = QES_MIDDLE_VAR;
extern uint32_t appTick;               // tick at an interval of 1ms,as Timer
KEY_MSG_t keymsg;                      //keymsg ，using FIFO to send status(S1,S2,QES)
bool QES_incflag = 0,QES_decflag = 0;  //Flag to elimination buffeting of keystroke of QES

// CCD
uint16_t CCDData[128];	// Linear CCD raw data

uint16_t NormalCCDData[128];	// Linear CCD raw data
uint16_t CCDDataToDraw[128], CCDDataToClear[128];	// Linear CCD data to OLED
uint8_t CCD2PC[260];	// data to be sent to PC
uint8_t Image_Data[120][188];	// MT9V034 DMA data receiver ROW*COL: 120*188

uint8_t Image_Use[MT9V034_IMAGEH2][MT9V034_IMAGEW2];	// resized CAM data to fit OLED
uint8_t Pixle[MT9V034_IMAGEH2][MT9V034_IMAGEW2];		// 0/1 CAM data

image_t Gauss,Binary,Road;

uint16_t CAM_DMA_Counter=0;
uint16_t threshold;
int32_t CCD_position,last_CCD_position;
float CCD_error;

uint8_t CmdDataLen;		// PC cmd data length counter
uint8_t CmdDataBuffer[cmdDataLenMax];	// buffer to hold cmd data from PC
bool CmdDataReady;		// new cmd data flag, 1: new cmd received
bool CamDataReady=0;	// MT9V034 new data frame ready in buffer
uint16_t DMALeftSize=0;	// test purpose
uint8_t CamLineCounter=0;


//MT9V034
#define USB_RING_BUFFER_SIZE	(128U)
#define cmdDataLenMax			(100U)
uint16_t threshold;

extern pic_t image;
uint16_t road_id;

//servo pid control
float servo_duty;
pid_t servo_pid = PID_CREATE(4, 0.3, 5, 0);           //舵机pid
//PHOTODIODE_t
PHOTODIODE_t Light;
extern double direction;

//MEMS
MMA7260_t MEMS;
float External_Force;
uint32_t MEMS_Status;
//DFS
#define LINE_MAX    (200)
uint32_t line[LINE_MAX][2];
uint32_t line_len;



void Get_Use_Image(image_t pic)
{
	uint16_t i=0, j=0, row=0, col=0;

	for(i=0; i<120; i=i+2)		// ROW
	{
		for(j=0; j<188; j=j+2)	// COL
		{
			pic[row][col] = Image_Data[i][j];
			col++;
		}
		col = 0;
		row++;
	}
}

uint8_t GetOTSU(uint8_t image[MT9V034_IMAGEH2][MT9V034_IMAGEW2])
{
	uint16_t i, j;
	uint32_t Amount = 0;
	uint32_t PixelBack = 0;
	uint32_t PixelIntegralBack = 0;
	uint32_t PixelIntegral = 0;
	uint32_t PixelIntegralFore = 0;
	uint32_t PixelFore = 0;
	float OmegaBack, OmegaFore, MicroBack, MicroFore, SigmaB, Sigma; // 类间方差;
	uint16_t MinValue, MaxValue;
	uint8_t Threshold = 0;
	uint8_t HistoGram[256];

	for (j=0; j<256; j++)  HistoGram[j] = 0; //初始化灰度直方图

	for (j=0; j<MT9V034_IMAGEH2; j++)
	{
		for (i = 0; i < MT9V034_IMAGEW2; i++)
		{
			HistoGram[image[j][i]]++; //统计灰度级中每个像素在整幅图像中的个数
		}
	}

	for (MinValue = 0; MinValue < 256 && HistoGram[MinValue] == 0; MinValue++) ;        //获取最小灰度的值
	for (MaxValue = 255; MaxValue > MinValue && HistoGram[MinValue] == 0; MaxValue--) ; //获取最大灰度的值

	if (MaxValue == MinValue)     return MaxValue;         // 图像中只有一个颜色
	if (MinValue + 1 == MaxValue)  return MinValue;        // 图像中只有二个颜色

	for (j = MinValue; j <= MaxValue; j++)    Amount += HistoGram[j];        //  像素总数

	PixelIntegral = 0;
	for (j = MinValue; j <= MaxValue; j++)
	{
		PixelIntegral += HistoGram[j] * j;//灰度值总数
	}
	SigmaB = -1;
	for (j = MinValue; j < MaxValue; j++)
	{
		PixelBack = PixelBack + HistoGram[j];   //前景像素点数
		PixelFore = Amount - PixelBack;         //背景像素点数
		OmegaBack = (float)PixelBack / Amount;//前景像素百分比
		OmegaFore = (float)PixelFore / Amount;//背景像素百分比
		PixelIntegralBack += HistoGram[j] * j;  //前景灰度值
		PixelIntegralFore = PixelIntegral - PixelIntegralBack;//背景灰度值
		MicroBack = (float)PixelIntegralBack / PixelBack;   //前景灰度百分比
		MicroFore = (float)PixelIntegralFore / PixelFore;   //背景灰度百分比
		Sigma = OmegaBack * OmegaFore * (MicroBack - MicroFore) * (MicroBack - MicroFore);//计算类间方差
		if (Sigma > SigmaB)                    //遍历最大的类间方差g //找出最大类间方差以及对应的阈值
		{
			SigmaB = Sigma;
			Threshold = j;
		}
	}
	return Threshold;                        //返回最佳阈值;

}

// threshold mode: 0 - OTSU; 1 - average
int Get_01_Value(image_t pic,uint8_t mode)
{
	uint16_t i=0, j=0;
	uint8_t Threshold;
	uint32_t tv=0;
	char txt[16];

	if (mode)
	{
		for (i=0; i<MT9V034_IMAGEH2; i++)
		{
			for (j=0; j<MT9V034_IMAGEW2; j++)
				{
					tv += pic[i][j];   //累加
				}
		}
		Threshold = tv/MT9V034_IMAGEH2/MT9V034_IMAGEW2;        	//求平均值,光线越暗越小，全黑约35，对着屏幕约160，一般情况下大约100
		Threshold = Threshold*8/10+10;	//此处阈值设置，根据环境的光线来设定
	}
	else
	{
		Threshold = GetOTSU(pic);	//大津法阈值
		Threshold = (uint8_t)(Threshold*0.5)+70;
	}
	sprintf(txt,"%03d", Threshold);
	OLED_P6x8Str(80, 1, (uint8_t *)txt);
    return Threshold;

}

void Image_Bianry(image_t origin_pic,image_t binary_pic,uint16_t threshold)
{
	uint16_t i=0, j=0,diff=8;
	for (i=0; i<MT9V034_IMAGEH2; i++)
    {
		for(j=0; j<MT9V034_IMAGEW2; j++)
		{
			if(origin_pic[i][j] > threshold) //数值越大，显示的内容越多，较浅的图像也能显示出来
				binary_pic[i][j] = 1;
			else
				binary_pic[i][j] = 0;
		}
	}

	for (i=1; i<MT9V034_IMAGEH2-1; i++)
	{
		for(j=1; j<MT9V034_IMAGEW2-1; j++)
		{
			if(binary_pic[i][j] == 0)
			{
				if(origin_pic[i][j]-origin_pic[i-1][j]<diff && abs(origin_pic[i][j]-origin_pic[i+1][j])<diff
				 && origin_pic[i][j]-origin_pic[i][j-1]<diff && abs(origin_pic[i][j]-origin_pic[i][j+1])<diff		)
				{
					binary_pic[i][j] = 1;
				}
			}
		}
	}
}

void Draw_CameraView(image_t pic)
{
	uint8_t i=0, j=0, temp=0;

	for(i=8;i<56;i+=8)//6*8=48行
	{
		OLED_Set_Pos(18, i/8+1);	//起始位置
		for(j=0; j<MT9V034_IMAGEW2; j++)  	//列数
		{
			temp = 0;
			if(pic[0+i][j]) temp |= 1;
			if(pic[1+i][j]) temp |= 2;
			if(pic[2+i][j]) temp |= 4;
			if(pic[3+i][j]) temp |= 8;
			if(pic[4+i][j]) temp |= 0x10;
			if(pic[5+i][j]) temp |= 0x20;
			if(pic[6+i][j]) temp |= 0x40;
			if(pic[7+i][j]) temp |= 0x80;
			OLED_WrDat(temp);
		}
	}
}


float dir;
uint8_t speed;
int main(void)
{
    Init_All();  //init_board_all
    CamDataReady = 0;
    CCD_position = 128/2;

    char speed_display[16];
    char dir_display[16];
    MT9V034_Init(50);
    while (1)
    {
    	if (CamDataReady)
    	{
    	   Get_Use_Image(Image_Use);	// resize data to fit OLED

    	   if (SW4())			// key S1 pressed down? If yes then use OTSU
    		   threshold=Get_01_Value(Image_Use,0);	// 0 - OTSU; 1 - average
    	   else
    		   threshold=Get_01_Value(Image_Use,1);	// 0 - OTSU; 1 - average

    	   ShowNumDEC(road_id);     //DEC show Var1 & 0x0F
    	   //threshold=70;
    	   //gauss
    	   fast_gauss_conv3(Image_Use,Gauss);

    	   Image_Bianry(Gauss,Binary,threshold);
    	   erode3(Binary,Pixle);
    	   dilate3(Pixle,Binary);
    	   road_judge(Binary);
    	   sprintf(speed_display,"%02d", speed);
    	   OLED_P6x8Str(5, 3, (uint8_t *)speed_display);

    	   sprintf(dir_display,"%02d", (int)(dir*16));
    	   OLED_P6x8Str(5, 5, (uint8_t *)dir_display);
    	   //Canny
    	   //Canny(Gauss,Pixle,75,150);


    	   /*DFS
    	   line_len = LINE_MAX;
    	   find_road_lines(Binary,line,&line_len);
    	   draw_road_line(Road,line_len);
           */
    	   Draw_CameraView(Binary);		// display on OLED

    	   if (!key_get(KEY2)) SendImage();	// key S2 pressed down? If yes send ImageData to PC
    	   CamDataReady = 0;
    	}

    }
    return 0;
}

void SendImage(void)
{
	uint8_t PcImageHeader[4];

	PcImageHeader[0] = 0x00;
	PcImageHeader[1] = 0xFF;
	PcImageHeader[2] = 0x01;
	PcImageHeader[3] = 0x01;

	//发送帧头标志
	UART_WriteBlocking(UART0_PERIPHERAL, (uint8_t *)PcImageHeader, 4U);
	//发送灰度图像
    UART_WriteBlocking(UART0_PERIPHERAL, (uint8_t *)Image_Data, 22560U);

}


//Send data to the host computer for visualization
void serial_out()
{
    static uint8_t data[13];
    data[0] = 0xAA;
    data[1] = 0xAA;
    data[2] = 0xF1;
    data[3] = 8;

    data[4] = BYTE3(CCD_error);
    data[5] = BYTE2(CCD_error);
    data[6] = BYTE1(CCD_error);
    data[7] = BYTE0(CCD_error);


    data[8] = BYTE3(CCD_position);
    data[9] = BYTE2(CCD_position);
    data[10] = BYTE1(CCD_position);
    data[11] = BYTE0(CCD_position);

    data[12] = 0;

    for (uint8_t *ptr = data; ptr < data + sizeof(data) - 1; ptr++)
    {
        data[12] += *ptr;
    }
    UART_WriteBlocking(UART0, (uint8_t *)&data, sizeof(data));
}


//Init_All
void Init_All(void)
{
   /* Init all hardware. */
   Board_InitAll();
   delay();

   OLED_Init();
   InitPITs();
   EnableIRQ(BOARD_RESa_IRQ);

   //OLED_P8x16Str(0, 0, " #Cyber-K66 G7#");

   delay();
   BEEP_OFF();
   ShowNumOFF();
   keymsg.status[KEY1] = 1;
   keymsg.status[KEY2] = 1;
   keymsg.status[QESP] = 1;
   // Header of CCD data frame, seekfree protocol
   CCD2PC[0] = 0x00;
   CCD2PC[1] = 0xFF;
   CCD2PC[2] = 0x01;
   CCD2PC[3] = 0x00;
   printf("Init Finished.\n");
   servo_duty=1580;
   delay();
}


/* PORTB_IRQn interrupt handler */
void DCAM_IRQHANDLER(void) {
  /* Get pin flags */
  uint32_t pin_flags = GPIO_PortGetInterruptFlags(GPIOB);

  /* Place your interrupt code here */
  // HREF on PTB11
  if (pin_flags & BOARD_INITPINS_CAMHREF_GPIO_PIN_MASK)
  {
	  GPIO_PortClearInterruptFlags(GPIOB, BOARD_INITPINS_CAMHREF_GPIO_PIN_MASK);
	  // Start DMA transfer on new line
	  	  DMA_CAM_Repeat(CamLineCounter);
	  	  CamLineCounter++;
	  	  if (CamLineCounter >= 120)
	  	  {
	  		  CamLineCounter = 0;
	  	  }
	  return;
  }
  // VSYN on PTB16
  if (pin_flags & BOARD_INITPINS_CAMVSYN_GPIO_PIN_MASK)
  {
  	  GPIO_PortClearInterruptFlags(GPIOB, BOARD_INITPINS_CAMVSYN_GPIO_PIN_MASK);

  	  //CAM_DMA_Counter = EDMA_GetRemainingMajorLoopCount(DMA_DMA_BASEADDR, DMA_CH0_DMA_CHANNEL);
  	  //DMA_CAM_Repeat();

  	  //DMALeftSize = EDMA_GetRemainingMajorLoopCount(DMA_DMA_BASEADDR, DMA_CH0_DMA_CHANNEL);

  	  CamDataReady = 1;
  	  //CamLineCounter = 0;
  	  //DMA_CAM_Repeat();

  	  CAM_DMA_Counter++;
  	  if ((CAM_DMA_Counter%50)==0)
  		  LED2_TOGGLE();

  	  return;
   }

  // QESa on PTB20
  if (pin_flags & BOARD_INITPINS_QESa_GPIO_PIN_MASK)
  {
	  GPIO_PortClearInterruptFlags(GPIOB, BOARD_INITPINS_QESa_GPIO_PIN_MASK);
	  if (QESA())
	  {
	         if (!QESB()) QES_incflag = 1; //maybe increase
	         if (QESB())  QES_decflag = 1; //maybe decrease
	   }
	   else
	  {
	       //Repeat judgment, elimination buffeting of keystroke of QES
	       if ( QESB() && QES_incflag == 1)  {QESVar++;}
	       if (!QESB() && QES_decflag == 1)  {QESVar--;}
	       QESVar=MINMAX(0,QESVar,255);          //constrain(0,255)
	       QES_decflag = 0;
	       QES_incflag = 0;
	   }
  }

  /* Clear pin flags */
  GPIO_PortClearInterruptFlags(GPIOB, pin_flags);

  /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F
     Store immediate overlapping exception return operation might vector to incorrect interrupt. */
  #if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
  #endif
}

void DMA_CAM_Repeat(uint8_t LineNumber)
//void DMA_CAM_Repeat()
{
	/* Channel CH0 status */
	//uint32_t status;

	//status = EDMA_GetChannelStatusFlags(DMA_DMA_BASEADDR, DMA_CH0_DMA_CHANNEL);
	//EDMA_ClearChannelStatusFlags(DMA_DMA_BASEADDR, DMA_CH0_DMA_CHANNEL, status);

	//EDMA_ResetChannel(DMA_DMA_BASEADDR, DMA_CH0_DMA_CHANNEL);
	//EnableIRQ(DMA_DMA_CH_INT_DONE_0_IRQN);
	//EDMA_InstallTCD(DMA_DMA_BASEADDR, DMA_CH0_DMA_CHANNEL, (edma_tcd_t *)&DMA_CH0_TCD0_config);

	//DMA_DMA_BASEADDR->TCD[DMA_CH0_DMA_CHANNEL].SADDR         = (uint32_t) 0x400FF050U;
	//DMA_DMA_BASEADDR->TCD[DMA_CH0_DMA_CHANNEL].DADDR         = (uint32_t) &Image_Data[0];
	DMA_DMA_BASEADDR->TCD[DMA_CH0_DMA_CHANNEL].DADDR         = (uint32_t) &Image_Data[LineNumber][0];
	DMA_DMA_BASEADDR->TCD[DMA_CH0_DMA_CHANNEL].CITER_ELINKNO = DMA_CITER_ELINKNO_CITER(188);
    /* Clear DONE bit first, otherwise ESG cannot be set */
	//DMA_DMA_BASEADDR->TCD[DMA_CH0_DMA_CHANNEL].CSR           = 0;
	//DMA_DMA_BASEADDR->TCD[DMA_CH0_DMA_CHANNEL].CSR           = DMA_CSR_START(0U) | DMA_CSR_INTMAJOR(1U) | DMA_CSR_INTHALF(0U) | DMA_CSR_DREQ(1U) | DMA_CSR_ESG(0U) | DMA_CSR_MAJORELINK(0U) | DMA_CSR_MAJORLINKCH(0) | DMA_CSR_BWC(3U);
	DMA_DMA_BASEADDR->TCD[DMA_CH0_DMA_CHANNEL].BITER_ELINKNO = DMA_BITER_ELINKNO_BITER(188);

	//EDMA_EnableChannelRequest(DMA_DMA_BASEADDR, DMA_CH0_DMA_CHANNEL);
	DMA_DMA_BASEADDR->SERQ = DMA_SERQ_SERQ(DMA_CH0_DMA_CHANNEL);

}

/* DMA0_DMA16_IRQn interrupt handler */
void DMA_DMA_CH_INT_DONE_0_IRQHANDLER(void) {

  /* Channel CH0 status */
  uint32_t CH0_status;

  /* Reading all flags of status register */
  CH0_status = EDMA_GetChannelStatusFlags(DMA_DMA_BASEADDR, DMA_CH0_DMA_CHANNEL);
  /* Clearing all flags of the channel status register */
  EDMA_ClearChannelStatusFlags(DMA_DMA_BASEADDR, DMA_CH0_DMA_CHANNEL, CH0_status);

  /* Place your code here */
  //CamDataReady = 1;
  //DMA_CAM_Repeat();

  /*
  CAM_DMA_Counter++;
  if ((CAM_DMA_Counter%50)==0)
	  LED2_TOGGLE();
	*/

  /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F
     Store immediate overlapping exception return operation might vector to incorrect interrupt. */
  #if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
  #endif
}







