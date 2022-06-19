
#ifndef _PID_H_
#define _PID_H_

#include "headfile.h"
#include "MK66F18.h"

#define MINMAX(x, l, u) MIN(MAX(x, l), u)

typedef struct pid_t{
	float kp;
	float ki;
	float kd;
	float _p;
	float _i;
	float _d;
	float _i_max;
}pid_t;

#define PID_CREATE(_kp, _ki, _kd, max_i) { \
	.kp = _kp, \
	.ki = _ki, \
	.kd = _kd, \
	._p = 0, \
	._i = 0, \
	._d = 0, \
	._i_max = max_i, \
}

float pid_solve(pid_t* pid, float target, float feedback);


#endif /* _PID_H_ */


