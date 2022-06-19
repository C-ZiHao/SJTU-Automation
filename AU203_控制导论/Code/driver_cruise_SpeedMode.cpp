
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
static void userDriverSetParam(float* cmdAcc, float* cmdBrake, float* cmdSteer, int* cmdGear);//ţɲת򣬵λ
static int InitFuncPt(int index, void* pt);

extern "C" int driver_cruise(tModInfo* modInfo) 
{
	memset(modInfo, 0, 10 * sizeof(tModInfo));
	modInfo[0].name = "driver_cruise";	// name of the module (short).
	modInfo[0].desc = "user module for CyberCruise";	// Description of the module (can be long).
	modInfo[0].fctInit = InitFuncPt;			// Init function.
	modInfo[0].gfId = 0;
	modInfo[0].index = 0;
	return 0;
}

// Module interface initialization.
static int InitFuncPt(int, void* pt)    
{
	tUserItf* itf = (tUserItf*)pt;
	itf->userDriverGetParam = userDriverGetParam;
	itf->userDriverSetParam = userDriverSetParam;
	return 0;
}

static float _midline[200][2];							
static float _yaw, _yawrate, _speed, _acc, _width, _rpm;
static int _gearbox;									

bool parameterSet = false;								
void PIDParamSetter();									


typedef struct Circle									
{														
	double r;											
	int sign;											
}circle;												


//********************PID parameters*************************//
double kp_s;	//kp for speed							     
double ki_s;	//ki for speed							     
double kd_s;	//kd for speed							     
double kp_d;	//kp for direction						     
double ki_d;	//ki for direction					    	 
double kd_d;	//kd for direction						     
// Direction Control Variables						         
double D_err;//direction error					             
double D_errDiff = 0;//direction difference(Differentiation) 
double D_errSum = 0;//sum of direction error(Integration)      
float MIN = 0;
// Speed Control Variables								     
circle c;												     
double expectedSpeed;   							     
double curSpeedErr;//speed error   		                     
double speedErrSum = 0;//sum of speed error(Integration)       
int startPoint;											     
int delta = 20;												
double dataOutCount = 0;
char* dataOut;
//***********************************************************//

//*******************Other parameters*******************//
const int topGear = 6;									
double tmp;												
bool flag = true;											
double offset = 0.16;									
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
int  distance = 1;
double disnum = 500;
double nowmin = 0;
bool wanflag;
bool leftflag;
double off = 1;
bool shaflag = 0;
int vpoint = 39;

void updateGear(int* cmdGear);													
double constrain(double lowerBoundary, double upperBoundary, double input);		
circle getR(float x1, float y1, float x2, float y2, float x3, float y3);		

static void userDriverGetParam(float midline[200][2], float yaw, float yawrate, float speed, float acc, float width, int gearbox, float rpm) {
	for (int i = 0; i < 200; ++i) _midline[i][0] = midline[i][0], _midline[i][1] = midline[i][1];
	_yaw = yaw;             
	_yawrate = yawrate;
	_speed = speed;       
	_acc = acc;          
	_width = width;      
	_rpm = rpm;     
	_gearbox = gearbox;        
}

static void userDriverSetParam(float* cmdAcc, float* cmdBrake, float* cmdSteer, int* cmdGear) {
	if (parameterSet == false)		
	{
		PIDParamSetter();
	}
	else
	{
		startPoint = max(float(_speed * _speed * 0.00096), float(_speed * 0.48));
		MIN = 501;
		for (int i = 5; i < startPoint; i++)
		{
			circle cirs = getR(_midline[i - 2][0], _midline[i - 2][1], _midline[i][0], _midline[i][1], _midline[i + 2][0], _midline[i + 2][1]);
			if (MIN > cirs.r)
			{
				MIN = cirs.r;
			}
		}
		delta2 = MIN - tmp3;
		tmp3 = MIN;
		
		if (MIN < 30)
		{
			expectedSpeed = constrain(93, 93, 22 * sqrt(double(MIN)));
		}
		else if (MIN < 50)
		{
			expectedSpeed = constrain(127, 140, 16 * sqrt(double(MIN)));
		}
		else if (MIN < 120)
		{
			expectedSpeed = constrain(160, 240, 18.4 * sqrt(double(MIN)));
		}
		else
		{
			expectedSpeed = constrain(270, 300, MIN);
		}
		curSpeedErr = expectedSpeed - _speed;
		speedErrSum = 0.1 * speedErrSum + curSpeedErr;
		if (curSpeedErr > 0)
		{
			if (abs(*cmdSteer) < 0.6)
			{
				*cmdAcc = constrain(0.0, 1.0, kp_s * curSpeedErr + ki_s * speedErrSum + offset);
				*cmdBrake = 0;
			}
			else if (abs(*cmdSteer) > 0.70)
			{
				*cmdAcc = 0.005 + offset;
				*cmdBrake = 0;
			}
			else
			{
				*cmdAcc = 0.11 + offset;
				*cmdBrake = 0;
			}

		}
		else if (curSpeedErr < 0)
		{

			*cmdBrake = constrain(0.0, 0.8, -0.065 * curSpeedErr / 5 + offset / 3);

			*cmdAcc = 0;

		}

		updateGear(cmdGear);
		if (dataOutCount * 20 / 1000 < 4)
		{
			if (_speed > 90)
			{
				shaflag = 0;
				offset = -0.03;
			}
		}
		vpoint = min(23 + 0.0017 * MIN * MIN, 45) * _width / 10.5;
		D_err = -atan2(_midline[vpoint][0], _midline[vpoint][1]);


		if (dataOutCount * 20 / 1000 > 5)
		{
			kp_d = 6.025;
			ki_d = 0.0;
			kd_d = 52.329;

			D_errSum = D_errSum + D_err;
			D_errSum2 = D_errSum;
		}
		else
		{
			kp_d = 7;
			ki_d = 0.0;
			kd_d = 6;
		}

		if (abs(_midline[0][0]) > _width / 10)
		{
			Tmp2 = dataOutCount;
		}

		if (dataOutCount < Tmp2 + 100 || abs(D_err)>0.1)
		{
			D_errSum = 0;
		}

		D_errDiff = D_err - Tmp;
		Tmp = D_err;
		D_errDiff2 = D_errDiff;

		if (_speed > 20)
		{
			*cmdSteer = constrain(-1.0, 1.0, kp_d * D_err + ki_d * D_errSum2 + kd_d * D_errDiff2);
		}
		else
		{
			*cmdSteer = 0;
			*cmdAcc = 1;
		}


		dataOutCount = dataOutCount + 1;
	}
}

void PIDParamSetter()
{

	kp_s = 0.07;
	ki_s = 0;
	kd_s = 0;
	kp_d = 1.35;
	ki_d = 0.151;
	kd_d = 0.10;
	parameterSet = true;

}

void updateGear(int* cmdGear)
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
	circle tmp = { r,sign };
	return tmp;
}

