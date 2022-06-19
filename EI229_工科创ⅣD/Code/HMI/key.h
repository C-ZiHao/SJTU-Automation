
#ifndef _KEY_H_
#define _KEY_H_

#include "headfile.h"
#include "MK66F18.h"

#define KEY_DOWN_DELAY_TIME   2      //confirm key-down time
#define KEY_HOLD_TIME         50     //press time
#define KEY_LONG_HOLD_TIME    100    //long press time
#define KEY_MSG_FIFO_SIZE       20   //Key message FIFO size

//SW_read
#define SW1() GPIO_PinRead(BOARD_INITPINS_SW1_GPIO, BOARD_INITPINS_SW1_PIN)
#define SW2() GPIO_PinRead(BOARD_INITPINS_SW2_GPIO, BOARD_INITPINS_SW2_PIN)
#define SW3() GPIO_PinRead(BOARD_INITPINS_SW3_GPIO, BOARD_INITPINS_SW3_PIN)
#define SW4() GPIO_PinRead(BOARD_INITPINS_SW4_GPIO, BOARD_INITPINS_SW4_PIN)

//Key_status
typedef enum
{
	KEY_DOWN  =   0,
	KEY_UP    =   1,
    KEY_HOLD  =   2,
	KEY_LONG_HOLD  =   3,
} KEY_STATUS_e;

typedef enum
{
    KEY1,
    KEY2,
	QESP,
    KEY_MAX,
} KEY_e;

typedef struct
{
    KEY_e           key;
    KEY_STATUS_e    status[KEY_MAX];
    uint32_t        count[KEY_MAX];
} KEY_MSG_t;

KEY_STATUS_e key_get(KEY_e key);
KEY_STATUS_e key_check(KEY_e key);
void send_key_msg(KEY_MSG_t keymsg);
uint8_t get_key_msg(KEY_MSG_t *keymsg);
#endif
