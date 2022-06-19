#ifdef _WIN32
#include <windows.h>
#endif
#include "driver_cruise.h"
#include "stdio.h"
#include <ostream>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>

#define PI 3.141592653589793238462643383279

static void userDriverGetParam(float midline[200][2], float yaw, float yawrate, float speed, float acc, float width, int gearbox, float rpm);
static void userDriverSetParam(float *cmdAcc, float *cmdBrake, float *cmdSteer, int *cmdGear);
static int InitFuncPt(int index, void *pt);

// Module Entry Point
extern "C" int driver_cruise(tModInfo *modInfo) 
{
	memset(modInfo, 0, 10 * sizeof(tModInfo));
	modInfo[0].name = "driver_cruise";				 // name of the module (short).
	modInfo[0].desc = "user module for CyberCruise"; // Description of the module (can be long).
	modInfo[0].fctInit = InitFuncPt;				 // Init function.
	modInfo[0].gfId = 0;
	modInfo[0].index = 0;
	return 0;
}

// Module interface initialization.
static int InitFuncPt(int, void *pt) 
{
	tUserItf *itf = (tUserItf *)pt;
	itf->userDriverGetParam = userDriverGetParam;
	itf->userDriverSetParam = userDriverSetParam;
	return 0;
}

//**********Global variables for vehicle states*********
static float _midline[200][2];							
static float _yaw, _yawrate, _speed, _acc, _width, _rpm; 
static int _gearbox;									 
//******************************************************

bool parameterSet = false; 
void PIDParamSetter();	  
typedef struct Circle 
{					  
	double r;		  
	int sign;		  
} circle;			 
//******************************************************

double kp_s; //kp for speed
double ki_s; //ki for speed
double kd_s; //kd for speed
double kp_d; //kp for direction
double ki_d; //ki for direction
double kd_d; //kd for direction
// Direction Control Variables
double D_err;		  //direction error
double D_errDiff = 0; //direction difference(Differentiation)
double D_errSum = 0;  //sum of direction error(Integration)
float MIN = 0;
// Speed Control Variables
circle c;
double expectedSpeed;
double curSpeedErr;		//speed error
double speedErrSum = 0; //sum of speed error(Integration)
int startPoint;
int delta = 20;
double dataOutCount = 0;
char *dataOut;
static float pointcoding[5];
static float pointspeed[5];

const int topGear = 6;
double tmp;
bool flag = true;
double offset = -0.09;
double Tmp = 0;
double Tmp2 = 0;
double k1 = 0;
double k2 = 0;
double D_errDiff2 = 0;
double D_errSum2 = 0;
int point = 5;
int minr = 0;
double delta2 = 0;
double tmp3 = 0;
double deltav = 0;
double tmp4 = 0;
double ddata = 0;
double tmp5 = 0;
double rmin = 0;
int distance = 1;
double disnum = 500;
double nowmin = 0;
bool wanflag;
bool leftflag;
double off = 1;
bool shaflag = 0;
int vpoint = 39;
static float findr[200];
float findminr = 500;
int k = 0;
int changetime = 0;
int minpoint = 0;
int tmpspeed = 0;
int tmppoint = 0;
int maxtmp = 0;
double k0 = 2;
double SUM = 0;
static double test[200];
double testsum = 0;
double testf = 0;
double tmpflag = 0;
double testflag = 10;
double ratio = 0;
double miderrp = 0;
double speedErrdiff = 0;

void updateGear(int *cmdGear);
double constrain(double lowerBoundary, double upperBoundary, double input);
circle getR(float x1, float y1, float x2, float y2, float x3, float y3);

static void userDriverGetParam(float midline[200][2], float yaw, float yawrate, float speed, float acc, float width, int gearbox, float rpm)
{

	for (int i = 0; i < 200; ++i)
		_midline[i][0] = midline[i][0], _midline[i][1] = midline[i][1]; 
	_yaw = yaw;
	_yawrate = yawrate;
	_speed = speed;
	_acc = acc;
	_width = width;
	_rpm = rpm;
	_gearbox = gearbox;
}

static void userDriverSetParam(float *cmdAcc, float *cmdBrake, float *cmdSteer, int *cmdGear)
{
	if (parameterSet == false)
	{
		PIDParamSetter();
	}
	else
	{
		startPoint = MAX(float(_speed * _speed * 0.00096), float(_speed * 0.715));
		MIN = 501;

		for (int i = 5; i < startPoint; i++)
		{
			circle cirs = getR(_midline[i - 2][0], _midline[i - 2][1], _midline[i][0], _midline[i][1], _midline[i + 2][0], _midline[i + 2][1]);
			findr[i] = cirs.r;
			if (MIN > cirs.r)
			{
				MIN = cirs.r;
			}
		}
		if (shaflag == 1)
		{
			if (MIN < 11)
			{
				expectedSpeed = constrain(30, 30, MIN);
			}
			else if (MIN < 41)
			{
				expectedSpeed = constrain(50, 50, MIN);
			}
			else if (MIN < 61)
			{
				expectedSpeed = constrain(60, 60, MIN);
			}
			else if (MIN < 120)
			{
				expectedSpeed = constrain(80, 80, MIN);
			}
			else if (MIN < 250)
			{
				expectedSpeed = constrain(90, 90, MIN);
			}
			else if (MIN < 350)
			{
				expectedSpeed = constrain(100, 100, MIN);
			}
			else
			{
				expectedSpeed = constrain(120, 120, MIN);
			}
		}
		else
		{
			if (MIN < 11)
			{
				expectedSpeed = constrain(30, 30, MIN);
			}
			else if (MIN < 31)
			{
				expectedSpeed = constrain(50, 50, MIN);
			}
			else if (MIN < 61)
			{
				expectedSpeed = constrain(70, 70, MIN);
			}
			else if (MIN < 91)
			{
				expectedSpeed = constrain(90, 90, MIN);
			}
			else if (MIN < 120)
			{
				expectedSpeed = constrain(110, 110, MIN);
			}
			else if (MIN < 250)
			{
				expectedSpeed = constrain(140, 140, MIN);
			}
			else if (MIN < 350)
			{
				expectedSpeed = constrain(150, 150, MIN);
			}
			else
			{
				expectedSpeed = constrain(170, 170, MIN);
			}
		}

		curSpeedErr = expectedSpeed - _speed;
		speedErrSum = 0.1 * speedErrSum + curSpeedErr;
		speedErrdiff = curSpeedErr - tmp4;
		tmp4 = curSpeedErr;
		if (curSpeedErr > 0)
		{
			if (abs(*cmdSteer) < 0.1)
			{
				if (abs(_yawrate) > 0.2 && dataOutCount > 500)
				{
					*cmdAcc = constrain(0.0, 0.4, kp_s * curSpeedErr + ki_s * speedErrSum + kd_s * speedErrdiff + offset);
				}
				else
				{
					*cmdAcc = constrain(0, 1.0, kp_s * curSpeedErr + ki_s * speedErrSum + kd_s * speedErrdiff + offset);
				}
				*cmdBrake = 0;
			}
			else if (abs(*cmdSteer) < 0.40)
			{
				if (abs(_yawrate) > 0.2 && dataOutCount > 500)
				{
					*cmdAcc = constrain(0, 0.4, kp_s * curSpeedErr + ki_s * speedErrSum + kd_s * speedErrdiff + offset); //0.005 + offset;
				}
				else
				{
					*cmdAcc = constrain(0, 0.8, kp_s * curSpeedErr + ki_s * speedErrSum + kd_s * speedErrdiff + offset);
				}
				*cmdBrake = 0;
			}
			else if (abs(*cmdSteer) > 0.70)
			{
				*cmdAcc = 0.11 + offset / 2;
				*cmdBrake = 0;
			}
			else
			{
				*cmdAcc = 0.3 + offset;
				*cmdBrake = 0;
			}
		}
		else if (curSpeedErr < 0)
		{
			*cmdBrake = constrain(0.0, 0.7, -kp_s * curSpeedErr / 5 - offset / 3);
			*cmdAcc = 0;
		}

		updateGear(cmdGear);

		if (dataOutCount * 20 / 1000 < 4 && _midline[0][0] > 0.005)
		{
			kp_d = 4;
			ki_d = 0;
			kd_d = 80;
		}
		else
		{
			if (shaflag == 1)
			{
				kp_d = 12.5;
				ki_d = 0;
				kd_d = 40;
				kp_s = 0.017;
				offset = 0.05;
			}
			else
			{
				kp_d = 7.6;
				ki_d = 0.4;
				kd_d = 40;
				offset = 0.05;
			}
			D_errSum = D_errSum + D_err;
			D_errSum2 = D_errSum;
		}

		k0 = 50;
		if (abs(_midline[0][0]) > 0.2 && dataOutCount * 20 / 1000 < 10)
		{
			k0 = 20;
		}
		if ((abs(_midline[0][0]) > 0.2 && shaflag == 0) || (abs(_midline[0][0]) > 0.05 && shaflag == 1))
		{
			Tmp2 = dataOutCount;
		}
		if (dataOutCount < Tmp2 + 10 && abs(_midline[0][0]) > 0.001)
		{
			D_errSum = 0;
		}
		D_err = _yaw - atan((k0 * (_midline[0][0])) / double(_speed + 1));

		D_errDiff = D_err - Tmp;
		Tmp = D_err;
		D_errDiff2 = D_errDiff;
		SUM = abs(_midline[0][0]) + SUM;
		if (_speed > 20)
		{
			*cmdSteer = constrain(-1.0, 1.0, kp_d * D_err + ki_d * D_errSum2 + kd_d * D_errDiff2);
		}
		else
		{
			*cmdAcc = 1;
		}
		if (dataOutCount < 100)
		{
			*cmdAcc = 1;
		}

		if (dataOutCount < 50)
		{
			if (_midline[0][0] < 0)
			{
				*cmdSteer = 1;
			}
			else
			{
				*cmdSteer = -1;
			}
			*cmdAcc = 1;
		}

		if (dataOutCount < 15)
		{
			*cmdSteer = 0;
			*cmdAcc = 1;
		}
		dataOutCount = dataOutCount + 1;

		testsum = 0;
		testf = 0;
		for (int n = 0; n < 199; n++)
		{
			test[n] = test[n + 1];
			testsum = testsum + test[n];
		}
		if (_gearbox == 1)
		{
			ratio = 3.82;
		}
		else if (_gearbox == 2)
		{
			ratio = 2.15;
		}
		else if (_gearbox == 3)
		{
			ratio = 1.56;
		}
		else if (_gearbox == 4)
		{
			ratio = 1.21;
		}
		else if (_gearbox == 5)
		{
			ratio = 0.97;
		}
		else if (_gearbox == 6)
		{
			ratio = 0.8;
		}
		test[199] = _rpm / _speed / ratio;

		testsum = testsum + test[199];
		for (int n = 0; n < 200; n++)
		{
			testf = testf + (test[n] - testsum / 200) * (test[n] - testsum / 200);
		}

		if (dataOutCount * 20 / 1000 > 10)
		{
			if ((testf > testflag) || (dataOutCount < double(tmpflag) + 3000))
			{
				if (testf > testflag)
				{
					tmpflag = dataOutCount;
					shaflag = 1;
					testflag = 1;
				}
			}
			else
			{
				shaflag = 0;
				testflag = 10;
			}
		}
		circle cirf = getR(_midline[0][0], _midline[0][1], _midline[5][0], _midline[5][1], _midline[10][0], _midline[10][1]);
		if (cirf.r > 50 && shaflag == 1)
		{
			tmpflag = tmpflag + 0.02;
		}
	}
}

void PIDParamSetter()
{

	kp_s = 0.017;
	ki_s = 0;
	kd_s = 0;
	kp_d = 1.35;
	ki_d = 0.151;
	kd_d = 0.10;
	parameterSet = true;
}

void updateGear(int *cmdGear)
{
	if (_gearbox == 1)
	{
		if (_speed >= 60 && topGear > 1)
		{
			*cmdGear = 2;
		}
		else
		{
			*cmdGear = 1;
		}
	}
	else if (_gearbox == 2)
	{
		if (_speed <= 45)
		{
			*cmdGear = 1;
		}
		else if (_speed >= 105 && topGear > 2)
		{
			*cmdGear = 3;
		}
		else
		{
			*cmdGear = 2;
		}
	}
	else if (_gearbox == 3)
	{
		if (_speed <= 90)
		{
			*cmdGear = 2;
		}
		else if (_speed >= 145 && topGear > 3)
		{
			*cmdGear = 4;
		}
		else
		{
			*cmdGear = 3;
		}
	}
	else if (_gearbox == 4)
	{
		if (_speed <= 131)
		{
			*cmdGear = 3;
		}
		else if (_speed >= 187 && topGear > 4)
		{
			*cmdGear = 5;
		}
		else
		{
			*cmdGear = 4;
		}
	}
	else if (_gearbox == 5)
	{
		if (_speed <= 173)
		{
			*cmdGear = 4;
		}
		else if (_speed >= 234 && topGear > 5)
		{
			*cmdGear = 6;
		}
		else
		{
			*cmdGear = 5;
		}
	}
	else if (_gearbox == 6)
	{
		if (_speed <= 219)
		{
			*cmdGear = 5;
		}
		else
		{
			*cmdGear = 6;
		}
	}
	else
	{
		*cmdGear = 1;
	}
}

double constrain(double lowerBoundary, double upperBoundary, double input)
{
	if (input > upperBoundary)
		return upperBoundary;
	else if (input < lowerBoundary)
		return lowerBoundary;
	else
		return input;
}

circle getR(float x1, float y1, float x2, float y2, float x3, float y3)
{
	double a, b, c, d, e, f;
	double r, x, y;

	a = 2 * (x2 - x1);
	b = 2 * (y2 - y1);
	c = x2 * x2 + y2 * y2 - x1 * x1 - y1 * y1;
	d = 2 * (x3 - x2);
	e = 2 * (y3 - y2);
	f = x3 * x3 + y3 * y3 - x2 * x2 - y2 * y2;
	x = (b * f - e * c) / (b * d - e * a);
	y = (d * c - a * f) / (b * d - e * a);
	r = sqrt((x - x1) * (x - x1) + (y - y1) * (y - y1));
	x = constrain(-1000.0, 1000.0, x);
	y = constrain(-1000.0, 1000.0, y);
	r = constrain(1.0, 500.0, r);
	int sign = (x > 0) ? 1 : -1;
	circle tmp = {r, sign};
	return tmp;
}
