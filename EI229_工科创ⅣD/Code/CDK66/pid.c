
/**
 * @file    pid.c
 * @brief   pid control
 * @data    2020/10/22
 */
#include <pid.h>

#include "pid.h"



//舵机控制--位置pid
float pid_solve(pid_t *pid, float target, float feedback)
{
    float error = target - feedback;
    pid->_d = error - pid->_p;
    pid->_i += error;
    pid->_i = MINMAX(pid->_i, -pid->_i_max, pid->_i_max);
    pid->_p = error;
    return pid->kp * pid->_p + pid->ki * pid->_i + pid->kd * pid->_d;
}
