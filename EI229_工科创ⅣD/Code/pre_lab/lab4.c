
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
void UART0_SERIAL_RX_TX_IRQHANDLER(void);
void PcCmdProcess(void);
void DCAM_IRQHANDLER(void);
void cal_dir(void);
void servo_control(float anchor_point);

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
uint8_t Image_Use[60][94];	// resized CAM data to fit OLED
uint8_t Pixle[60][94];		// 0/1 CAM data
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


//servo pid control
float servo_duty;
pid_t servo_pid = PID_CREATE(4, 0.3, 5, 0);           //舵机pid

PHOTODIODE_t Light;
extern double direction;

//MEMS
MMA7260_t MEMS;
float External_Force;
uint32_t MEMS_Status;




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




int main(void)
{
    Init_All();  //init_board_all
    CamDataReady = 0;
    CCD_position = 128/2;
    uint32_t tempInt = 0;
    while (1)
    {
        Draw_LinearView();
        BOARD_I2C_GPIO(1<<(int)(8-direction*8));



      /*
       for(tempInt=0; tempInt<128; tempInt++)
      {
           CCD2PC[tempInt*2+4] = CCDData[tempInt] >> 8;	// Upper byte
           CCD2PC[tempInt*2+5] = CCDData[tempInt] & 0XFF;	// Lower byte
       }
       UART_WriteBlocking(UART0_PERIPHERAL, CCD2PC, 260U);
       */
       //printf("error:%f,thre:%d,position:%d\n",CCD_error,threshold,CCD_position);

    }
    return 0;
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

   OLED_P8x16Str(0, 0, " #Cyber-K66 G7#");

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
  if (pin_flags & BOARD_INITPINS_CAMDHREF_PIN_MASK)
  {
	  GPIO_PortClearInterruptFlags(GPIOB, BOARD_INITPINS_CAMDHREF_PIN_MASK);
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







