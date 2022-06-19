
/**
 * @file    ftm_timer.c
 * @brief   ftm timer,use pit finally
 * @data    2020/9/29
 */

#include <servo.h>

double k = 3000;
double d1,d2,d3,d0;
double angle,angle1,angle2;
double error0;

double direction;
extern PHOTODIODE_t Light;
extern float servo_duty;
extern pid_t servo_pid;           //舵机pid

void servo_control(float anchor_point) //输入偏差
{
    servo_duty = SERVO_DUTY_MID - pid_solve(&servo_pid, anchor_point, 0);   //舵机pid
    servo_duty = MIN(MAX(servo_duty, SERVO_DUTY_MIN), SERVO_DUTY_MAX);
    Update_ServoUS(kFTM_Chnl_0, servo_duty);
    Update_ServoUS(kFTM_Chnl_1, 3000-servo_duty);
}


void cal_dir(void)
{
	if ((Light.L1>=15||Light.L2>=15)&&(Light.L1<4096&&Light.L2<4096))//亮度合适
	{
		/*
		d1=sqrt(k/Light.L1);
		d2=sqrt(k/Light.L2);
		d0 = 1.7;//两个光敏三极管的距离
		if(d1+d2>d3&&d2+d3>d1&&d3+d1>d2)//几何解算，估测的两个光敏三极管和光源的距离，和两个光敏三极管的距离，三边可以构成三角形
	    {
		  angle1 = acos((d1*d1 + d0 * d0 - d2 * d2) / 2 / d1 / d0);
	      d3 = sqrt(d1*d1 + d0 * d0 / 4 - cos(angle1)*d1*d0);
	      angle2 = asin(d0/2/d3*sin(angle1));
	      angle = angle1 +angle2;
	      error0=(2/3.1415926*angle-1);
	      direction = error0;
	      Light.error=(SERVO_DUTY_MAX-SERVO_DUTY_MIN)/2*error0;
	    }
		else//估测的两个光敏三极管和光源的距离，和两个光敏三极管的距离，三边不能构成三角形，不能进行几何解算，用差比和算法顶替
		{
			Light.error=(SERVO_DUTY_MAX-SERVO_DUTY_MIN)/2*(Light.L1-Light.L2)/(Light.L1+Light.L2);
	        direction = 1.6*(Light.L1-Light.L2)/(Light.L1+Light.L2);
	        direction= MIN(MAX(direction, -0.99), 0.99);
		}
		*/

	}
	Light.error=250*(Light.L1-Light.L2)/(Light.L1+Light.L2);
	direction = 1.4*(Light.L1-Light.L2)/(Light.L1+Light.L2);
    direction= MIN(MAX(direction, -0.99), 0.99);
}

void static inline Update_ServoUS(uint8_t ChanNo, uint32_t DutyCycleUS) {
	uint32_t cv, mod;

	if ((ChanNo<2) && (DutyCycleUS<2500) && (DutyCycleUS>500))
	{
		// mod value mapped to 10ms|10000us period (100Hz)
		mod = FTM3_PERIPHERAL->MOD;
		cv = (mod * DutyCycleUS) / 10000U;
		if (cv >= mod)
		{
			cv = mod + 1U;
		}
		FTM3_PERIPHERAL->CONTROLS[ChanNo].CnV = cv;
	    FTM_SetSoftwareTrigger(FTM3_PERIPHERAL, true);
	}
	return;
}
