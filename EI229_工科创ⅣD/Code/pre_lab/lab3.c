
/**
 * @file      CDK66_Ref1.c
 * @brief     Lab3 of Engineering practice and technological innovation.
 *            mission : Familiar with the use of servo control
 *                      Use the data collected by PHOTODIODE to control the status of the servo
 * @Features  a. Complete the basic library, conducive to follow-up experiments
 *            b. Use serial port to debug and visualize on the host computer
 *            c. Use filtering algorithm to ensure the accuracy of PHOTODIODE data
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
void intquicksort(volatile int32_t array[], int maxlen, int begin, int end);
void cal_dir(void);
void servo_control(float anchor_point);

//KEY
#define QES_MIDDLE_VAR 128
volatile uint16_t QESVar = QES_MIDDLE_VAR;
extern uint32_t appTick;               // tick at an interval of 1ms,as Timer
KEY_MSG_t keymsg;                      //keymsg ，using FIFO to send status(S1,S2,QES)
bool QES_incflag = 0,QES_decflag = 0;  //Flag to elimination buffeting of keystroke of QES

// CCD
uint8_t CCD2PC[260];	// data to be sent to PC
Board_Analog_t AnalogIn;

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

    data[4] = BYTE3(Light.L1);
    data[5] = BYTE2(Light.L1);
    data[6] = BYTE1(Light.L1);
    data[7] = BYTE0(Light.L1);


    data[8] = BYTE3(Light.L2);
    data[9] = BYTE2(Light.L2);
    data[10] = BYTE1(Light.L2);
    data[11] = BYTE0(Light.L2);

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
    while (1)
    {
    	//电池电压读取和显示

    	/*
    	uint16_t volt=MEM_convertB(ADC0,7U)*1000*3.3*5/4096;
    	OLED_uint16(0, 2, volt);
    	OLED_P8x16Str(50, 2, "mV");
    	*/

    	serial_out();

    	BOARD_I2C_GPIO(1<<(int)(8-direction*8));//光柱指示方向

    	OLED_P8x16Str(0, 0, " #Cyber-K66 G7#");
    	OLED_P8x16Str(0, 2, "L1:");
    	OLED_P8x16Str(0, 4, "L2:");
    	OLED_uint16(24, 2, Light.L1);
    	OLED_uint16(24, 4, Light.L2);
    	if(Light.L1>=4096||Light.L2>=4096)    {OLED_P8x16Str(0, 6, "TOO LIGHT!");BOARD_I2C_GPIO((1<<16)-1);}
    	else if(Light.L1<10 && Light.L2<10)  {OLED_P8x16Str(0, 6, "TOO DARK!");BOARD_I2C_GPIO((1<<16)-1);}
    	else OLED_P8x16Str(0, 6, "               ");

    	printf("Light_right:%d ,Light_left:%d ,error:%d\n",Light.L1,Light.L2,Light.error);

    	systick_delay_ms(20);
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
void intquicksort(volatile int32_t array[], int maxlen, int begin, int end)
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

        intquicksort(array, maxlen, begin, i);
        intquicksort(array, maxlen, j, end);
    }
}



