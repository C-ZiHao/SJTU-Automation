

#ifndef _SERVO_h
#define _SERVO_h

#include "headfile.h"
#include "pid.h"



#define SERVO_DUTY_MIN 700
#define SERVO_DUTY_MID 1580
#define SERVO_DUTY_MAX 2460

void servo_control(float anchor_point);
void static inline Update_ServoUS(uint8_t ChanNo, uint32_t DutyCycleUS);
void cal_dir(void);

#endif
