
/**
 * @file    key.c
 * @brief   key read
 * @data    2020/10/6
 */
#include <key.h>

typedef enum
{
    KEY_MSG_EMPTY,
    KEY_MSG_NORMAL,
    KEY_MSG_FULL,
} key_msg_e;
KEY_MSG_t           key_msg[KEY_MSG_FIFO_SIZE];
volatile uint8_t      key_msg_front = 0, key_msg_rear = 0;
volatile uint8_t      key_msg_flag = KEY_MSG_EMPTY;

//-------------------------------------------------------------------------------------------------------------------
//  @brief         Get the key status (only read)
//  @return        KEY_STATUS_e
//  @data    2020/10/6
//  Sample usage:  Key1_status = key_get(KEY1)
//-------------------------------------------------------------------------------------------------------------------
KEY_STATUS_e key_get(KEY_e key)
{
    if(key==KEY1)      return GPIO_PinRead(BOARD_INITPINS_KEY1_GPIO, BOARD_INITPINS_KEY1_PIN);
    else if(key==KEY2) return GPIO_PinRead(BOARD_INITPINS_KEY2_GPIO, BOARD_INITPINS_KEY2_PIN);
    else  return GPIO_PinRead(BOARD_INITPINS_QESp_GPIO, BOARD_INITPINS_QESp_PIN);
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief         Get the key status (simply delay to elimination buffeting)
//  @return        KEY_STATUS_e
//  @data          2020/10/6
//  Sample usage:  Key1_status = key_check(KEY1)
//-------------------------------------------------------------------------------------------------------------------
KEY_STATUS_e key_check(KEY_e key)
{
    if(key_get(key) == KEY_DOWN)
    {
    	//delay();
        systick_delay_ms(KEY_DOWN_DELAY_TIME);
        if(key_get(key) == KEY_DOWN)
        {
            return KEY_DOWN;
        }
    }
    return KEY_UP;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief         Send key message to FIFO
//  @return        void
//  @data          2020/10/6
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void send_key_msg(KEY_MSG_t keymsg)
{
    uint8_t tmp;
    if(key_msg_flag == KEY_MSG_FULL)
    {
        return ;
    }
    key_msg[key_msg_rear].key = keymsg.key;
    key_msg[key_msg_rear].status[keymsg.key] = keymsg.status[keymsg.key];

    key_msg_rear++;

    if(key_msg_rear >= KEY_MSG_FIFO_SIZE)
    {
        key_msg_rear = 0;                       //start over
    }

    tmp = key_msg_rear;
    if(tmp == key_msg_front)                   //full
    {
        key_msg_flag = KEY_MSG_FULL;
    }
    else
    {
        key_msg_flag = KEY_MSG_NORMAL;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief         Get key press message from FIFO
//  @return        1-acquisition, 0-the key message is not obtained
//  @data          2020/10/6
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------

uint8_t get_key_msg(KEY_MSG_t *keymsg)
{
    uint8_t tmp;

    if(key_msg_flag == KEY_MSG_EMPTY)               //The key message FIFO is empty, return 0 directly
    {
        return 0;
    }

    keymsg->key = key_msg[key_msg_front].key;       //Get the key value from the head of the FIFO
    keymsg->status[keymsg->key] = key_msg[key_msg_front].status[keymsg->key]; //Get the button type from the FIFO head

    key_msg_front++;                                //The first pointer of the FIFO team is incremented by 1 to point to the next message

    if(key_msg_front >= KEY_MSG_FIFO_SIZE)          //If the FIFO pointer overflows at the head of the queue, it starts counting from 0
    {
        key_msg_front = 0;                          //Start over
    }

    tmp = key_msg_rear;
    if(key_msg_front == tmp)                        //Compare whether the head and tail of the team are the same, the same means that the FIFO is empty
    {
        key_msg_flag = KEY_MSG_EMPTY;
    }
    else
    {
        key_msg_flag = KEY_MSG_NORMAL;
    }

    return 1;
}
