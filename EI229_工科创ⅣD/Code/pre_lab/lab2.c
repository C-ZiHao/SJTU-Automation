
/**
 * @file      CDK66_Ref1.c
 * @brief     Lab2 of Engineering practice and technological innovation.
 *            mission : Familiar with the use of serial port
 *                      Use the data collected by MEM to judge the status of the board
 * @Features  a. Complete the basic library, conducive to follow-up experiments
 *            b. Use serial port to debug and visualize on the host computer
 *            c. Use filtering algorithm to ensure the accuracy of MEMS data
 *            d. Use normalization algorithm to ensure the generalization of the algorithm
 *            e. Analyze the force of the board to determine the state
 * @TeamNum   G7
 * @Team      @Chai_Zihao @Xu_Yanxuan @Chen_Dengjian @Lai_Wentao
 * @data      2020/10/16
 */
#include "headfile.h"


#define MINMAX(x, l, u) MIN(MAX(x, l), u)

#define BYTE0(dwTemp) (*(char *)(&dwTemp))
#define BYTE1(dwTemp) (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp) (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp) (*((char *)(&dwTemp) + 3))


void Init_All(void);
void UART0_SERIAL_RX_TX_IRQHANDLER(void);
void PcCmdProcess(void);
void DCAM_IRQHANDLER(void);

//KEY
#define QES_MIDDLE_VAR 128
volatile uint16_t QESVar = QES_MIDDLE_VAR;
extern uint32_t appTick;               // tick at an interval of 1ms,as Timer
KEY_MSG_t keymsg;                      //keymsg ，using FIFO to send status(S1,S2,QES)
bool QES_incflag = 0,QES_decflag = 0;  //Flag to elimination buffeting of keystroke of QES

//MEMS
MMA7260_t MEMS;
float External_Force;
uint32_t MEMS_Status;
char MEMSValueX[] = "ABCD";
char MEMSValueY[] = "ABCD";
char MEMSValueZ[] = "ABCD";

// USB debug port: UART0
#define USB_RING_BUFFER_SIZE	(128U)
#define cmdDataLenMax			(100U)
uint8_t UsbRingBuffer[USB_RING_BUFFER_SIZE];
uint16_t g_bufferWriteByteCnt;
volatile uint16_t txIndexUSB;	/* Index of the debug port data to send out. */
volatile uint16_t rxIndexUSB;	/* Index of the memory to save new arrived debug port data. */
uint8_t CmdDataLen;		// PC cmd data length counter
uint8_t CmdDataBuffer[cmdDataLenMax];	// buffer to hold cmd data from PC
bool CmdDataReady;		// new cmd data flag, 1: new cmd received


//Send data to the host computer for visualization
void serial_out()
{
    static uint8_t data[25];
    data[0] = 0xAA;
    data[1] = 0xAA;
    data[2] = 0xF1;
    data[3] = 20;
    data[4] = BYTE3(MEMS.x);
    data[5] = BYTE2(MEMS.x);
    data[6] = BYTE1(MEMS.x);
    data[7] = BYTE0(MEMS.x);
    data[8] = BYTE3(MEMS.y);
    data[9] = BYTE2(MEMS.y);
    data[10] = BYTE1(MEMS.y);
    data[11] = BYTE0(MEMS.y);

    data[12] = BYTE3(MEMS.z);
    data[13] = BYTE2(MEMS.z);
    data[14] = BYTE1(MEMS.z);
    data[15] = BYTE0(MEMS.z);

    int32_t force=(int32_t)(External_Force);
    data[16] = BYTE3(force);
    data[17] = BYTE2(force);
    data[18] = BYTE1(force);
    data[19] = BYTE0(force);

    data[20] = BYTE3(MEMS_Status);
    data[21] = BYTE2(MEMS_Status);
    data[22] = BYTE1(MEMS_Status);
    data[23] = BYTE0(MEMS_Status);
    data[24] = 0;

    for (uint8_t *ptr = data; ptr < data + sizeof(data) - 1; ptr++)
    {
        data[24] += *ptr;
    }
    UART_WriteBlocking(UART0, (uint8_t *)&data, sizeof(data));
}


int main(void)
{

    Init_All();  //init_board_all
    while (1)
    {

         serial_out();
    	 External_Force=Fsqrt((float)(MEMS.x*MEMS.x+MEMS.y*MEMS.y+MEMS.z*MEMS.z));

    	 //printf("force: %d\n",(uint16_t)(External_Force));
    	 if(abs(External_Force-128)<15)
    	 {
    	     MEMS_Status=0;
    	     BEEP_OFF();
    	     printf("Static!\n");
    	 }
    	 else if(abs(External_Force)<45)
    	 {
    	     MEMS_Status=2;
    	     BEEP_ON();
    	     printf("Free falling!\n");
    	 }
    	 else
    	 {
    	     MEMS_Status=1;
    	     BEEP_OFF();
    	     printf("Moving!\n");
    	 }
    	 //printf("status: %d\n",MEMS_Status);
    	 //OLED show
    	 sprintf (MEMSValueX, "%04hd", MEMS.x);
    	 sprintf (MEMSValueY, "%04hd", MEMS.y);
    	 sprintf (MEMSValueZ, "%04hd", MEMS.z);

    	 OLED_P8x16Str(28,2,MEMSValueX);
    	 OLED_P8x16Str(28,4,MEMSValueY);
    	 OLED_P8x16Str(28,6,MEMSValueZ);
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
   OLED_P8x16Str(0, 2, " X:");
   OLED_P8x16Str(0, 4, " Y:");
   OLED_P8x16Str(0, 6, " Z:");

   delay();
   BEEP_OFF();
   ShowNumOFF();
   keymsg.status[KEY1] = 1;
   keymsg.status[KEY2] = 1;
   keymsg.status[QESP] = 1;
   printf("Init Finished.\n");
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
	  // Start DMA transfer
	  return;
  }
  // VSYN on PTB16
  if (pin_flags & BOARD_INITPINS_CAMVSYN_PIN_MASK)
  {
	  GPIO_PortClearInterruptFlags(GPIOB, BOARD_INITPINS_CAMVSYN_PIN_MASK);
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


void PcCmdProcess() {
    int year;	/*!< Range from 1970 to 2099.*/
    int month;	/*!< Range from 1 to 12.*/
    int day;	/*!< Range from 1 to 31 (depending on month).*/
    int hour;	/*!< Range from 0 to 23.*/
    int minute;	/*!< Range from 0 to 59.*/
    int second;	/*!< Range from 0 to 59.*/
    //rtc_datetime_t PCTIME;
    uint8_t errMsg;


	if (CmdDataLen == 27U)	// "#cmd01data20201014-190000\r\n"
	{
		printf("Command from PC received!\r\n");
		sscanf(CmdDataBuffer, "#cmd01data%4d%2d%2d-%2d%2d%2d",
				&year, &month, &day, &hour, &minute, &second);
		printf("RTC: %04hd-%02hd-%02hd %02hd:%02hd:%02hd\r\n",
				year, month, day, hour, minute, second);
	}
	else
	{
		printf("Invalid command!\r\n");
	}
}


/* UART0_RX_TX_IRQn interrupt handler */
void UART0_SERIAL_RX_TX_IRQHANDLER(void) {
	uint8_t data;
	uint32_t intStatus;

	/* Reading all interrupt flags of status registers */
	intStatus = UART_GetStatusFlags(UART0_PERIPHERAL);

	/* If new data arrived. */
	if ((kUART_RxDataRegFullFlag | kUART_RxOverrunFlag) & UART_GetStatusFlags(UART0_PERIPHERAL))
	{
		data = UART_ReadByte(UART0_PERIPHERAL);


		/* PC command data framing */
		/* RTC calibrate example: "#cmd01data20201014-190000\r\n" */
		if (data == '#')		// start of a data frame?
		{

			CmdDataLen = 0;		// reset cmd data counter
			CmdDataBuffer[0] = data;
		}
		else
		{
			CmdDataLen++;
			if (CmdDataLen < cmdDataLenMax)
			{
				CmdDataBuffer[CmdDataLen] = data;
				if (data == '\n')	// end of a data frame?
				{
					printf("1111");
					CmdDataReady = 1;	// a complete cmd data frame received
					CmdDataLen++;
					PcCmdProcess();		// cmd processing
				}
			}
			else	// no cmd data frame received while data buffer overflow
			{
				CmdDataLen = 0;
			}
		}

		/* If ring buffer is not full, add data to ring buffer. */
		if (((rxIndexUSB + 1) % USB_RING_BUFFER_SIZE) != txIndexUSB)
		{
			UsbRingBuffer[rxIndexUSB] = data;
			rxIndexUSB++;
			rxIndexUSB %= USB_RING_BUFFER_SIZE;
		}
	}
#if defined __CORTEX_M && (__CORTEX_M == 4U)
	__DSB();
#endif
}
