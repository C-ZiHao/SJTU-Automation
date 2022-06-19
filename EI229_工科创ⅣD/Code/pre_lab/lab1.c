
/**
 * @file      CDK66_Ref1.c
 * @brief     Lab1 of Engineering practice and technological innovation.
 *            mission1 : Press S1 to start the timer, use S2 to pause, long press S1 to clear the number
 *                       Set a specific value, and beep when the counter reaches
 *                       Display in digital tube,OLED and light beam
 *            mission2 : Rotary digital switch to change Var2, use QES to return to 128
 *                       Display in digital tube,OLED and light beam
 * @Features  a. Complete the basic library, conducive to follow-up experiments
 *            b. Use FIFO to elimination buffeting of keystroke of button
 *            c. Use confirm flag to elimination buffeting of keystroke of QES Rotary digital switch
 * @data    2020/10/6
 */
#include "headfile.h"

//#define MIN(x, y) ((x) < (y) ? (x) : (y))
//#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MINMAX(x, l, u) MIN(MAX(x, l), u)

#define Assign_Var 12   //set the assign num
#define QES_MIDDLE_VAR 128

void BOARD_RESa_IRQ_HANDLER(void);
void Init_All(void);
void Ref1_mission1(void);
void Ref1_mission2(void);

volatile uint16_t Var1 = 0,Var2 = QES_MIDDLE_VAR;
extern uint32_t appTick;               // tick at an interval of 1ms,as Timer
KEY_MSG_t keymsg;                      //keymsg ，using FIFO to send status(S1,S2,QES)
volatile bool loop_flag = 0;           //loop stop flag of mission1,refresh in pit of key
uint16_t mode=1, premode=2;            //mode to change mission ,set premode to save OLED refresh time
bool QES_incflag = 0,QES_decflag = 0;  //Flag to elimination buffeting of keystroke of QES

int main(void)
{
    Init_All();  //init_board_all
    while (1)
    {
    	if (get_key_msg(&keymsg) == 1) //get key_message from FIFO，0-down，1-up，2-press，3-long_press
    	{
    	   printf("key_msg:%d\n key1_status:%d, key2_status:%d, QES_status:%d\n key1_count1:%d, key2_count:%d, QESP_count:%d\n",
    	      keymsg.key, keymsg.status[KEY1], keymsg.status[KEY2], keymsg.status[QESP], keymsg.count[KEY1], keymsg.count[KEY2], keymsg.count[QESP]);
    	}

        if(mode==1)   {Ref1_mission1();} //run mission
        else          {Ref1_mission2();}
    }
    return 0;
}

/* PORTB_IRQn interrupt handler ,interrupt either rise and fall*/
void BOARD_RESa_IRQ_HANDLER(void)
{
    GPIO_PortClearInterruptFlags(BOARD_INITPINS_QESa_GPIO, BOARD_INITPINS_QESa_GPIO_PIN_MASK);
    if (QESA())
    {
       if (!QESB()) QES_incflag = 1; //maybe increase
       if (QESB())  QES_decflag = 1; //maybe decrease
    }
    else
    {
     //Repeat judgment, elimination buffeting of keystroke of QES
     if ( QESB() && QES_incflag == 1 && mode==2)  {Var2++;}
     if (!QESB() && QES_decflag == 1 && mode==2)  {Var2--;}
     Var2=MINMAX(0,Var2,255);          //constrain(0,255)
     QES_decflag = 0;
     QES_incflag = 0;
    }
#if defined __CORTEX_M && (__CORTEX_M == 4U)
        __DSB();
#endif
}
void Ref1_mission2(void)
{
	if(premode!=mode)     //only run when mode-change,to save OLED refresh time
	{
		OLED_Fill(0x00);     //clear oled
		OLED_P8x16Str(0, 0, "## Mission2 ###");
		OLED_P8x16Str(0, 2, "var2=");
		oled_uint16(40, 2, Var2);  //OLED show Var2
		OLED_P6x8Str(0, 4, "s1:run,s2:stop");
		OLED_P6x8Str(0, 5, "qes:recount");
		OLED_P6x8Str(0, 6, "sw1:mode1");
	}
	if (keymsg.status[QESP] == KEY_DOWN) {Var2= QES_MIDDLE_VAR;} // QES key down ,reset
	ShowNumDEC(Var2);          //DEC show Var2
	BOARD_I2C_GPIO(Var2);      //light show Var2
	oled_uint16(40, 2, Var2);  //OLED show Var2
}

void Ref1_mission1(void)
{
	if(premode!=mode)   //only run when mode-change,to save OLED refresh time
	{
		OLED_Fill(0x00);
		OLED_P8x16Str(0, 0, "## Mission1 ###");
		OLED_P8x16Str(0, 2, "var1=");
		oled_uint16(40, 2, Var1 );
		OLED_P6x8Str(0, 4, "s1:run,s2:stop");
		OLED_P6x8Str(0, 5, "sw1:mode2");
		OLED_P6x8Str(0, 6, "press s1:recount");
	}

    if (loop_flag)
    {
        if ((appTick % 100) == 0)
        {
            Var1++;
            if (Var1 == Assign_Var) { BEEP_ON();}  //BEEP
            else { BEEP_OFF();}
            ShowNumDEC(Var1 & 0X0F);     //DEC show Var1 & 0x0F
            BOARD_I2C_GPIO(Var1);        //light show Var1
            oled_uint16(40, 2, Var1 );   //OLED show Var1
        }
        if(keymsg.status[KEY1]==KEY_HOLD)  {Var1=0;}
    }
}



void Init_All(void)
{
   /* Init all hardware. */
   Board_InitAll();
   delay();

   OLED_Init();
   InitPITs();
   EnableIRQ(BOARD_RESa_IRQ);

   OLED_P8x16Str(0, 0, "## Cyber-K66 ###");

   BEEP_ON();
   delay();
   BEEP_OFF();
   ShowNumOFF();
   keymsg.status[KEY1] = 1;
   keymsg.status[KEY2] = 1;
   keymsg.status[QESP] = 1;
   printf("Init Finished.\n");
}
