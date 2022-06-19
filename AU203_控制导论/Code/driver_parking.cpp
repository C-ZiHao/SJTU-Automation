
#ifdef _WIN32
#include <windows.h>
#endif

#include <math.h>
#include "driver_parking.h"
#include <cmath>

static void userDriverGetParam(float lotX, float lotY, float lotAngle, bool bFrontIn, float carX, float carY, float caryaw, float midline[200][2], float yaw, float yawrate, float speed, float acc, float width, int gearbox, float rpm);
static void userDriverSetParam(bool *bFinished, float *cmdAcc, float *cmdBrake, float *cmdSteer, int *cmdGear);
static int InitFuncPt(int index, void *pt);

// Module Entry Point
extern "C" int driver_parking(tModInfo *modInfo)
{
	memset(modInfo, 0, 10 * sizeof(tModInfo));
	modInfo[0].name = "driver_parking";				  // name of the module (short).
	modInfo[0].desc = "user module for CyberParking"; // Description of the module (can be long).
	modInfo[0].fctInit = InitFuncPt;				  // Init function.
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
	printf("OK!\n");
	return 0;
}

static float _midline[200][2];
static float _yaw, _yawrate, _speed, _acc, _width, _rpm, _lotX, _lotY, _lotAngle, _carX, _carY, _caryaw;
static int _gearbox;
static bool _bFrontIn;

bool parameterSet = false;
void PIDParamSetter();

typedef struct Circle
{
	double r;
	int sign;
} circle;

//********************PID parameters*************************//
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
circle c;
double expectedSpeed;
double curSpeedErr;		//speed error
double speedErrSum = 0; //sum of speed error(Integration)
int startPoint;
int delta = 20;
double dataOutCount = 0;
char *dataOut;
int dotflag = 0;
double parkR = 8;
static float findr[200];
float findminr = 500;
double Tmp = 0;
double cnt = 0;
double successflag = 0;
double distance = 0;
double testflag = 0;
double offset = 2;
double err = 0;
double errdipp = 0;
double kp = 1;
double ki = 1;
double errtmp = 0;
double cnttmp = 0;
double constrain(double lowerBoundary, double upperBoundary, double input);
circle getR(float x1, float y1, float x2, float y2, float x3, float y3);

static void userDriverGetParam(float lotX, float lotY, float lotAngle, bool bFrontIn, float carX, float carY, float caryaw, float midline[200][2], float yaw, float yawrate, float speed, float acc, float width, int gearbox, float rpm)
{
	_lotX = lotX;
	_lotY = lotY;
	_lotAngle = lotAngle;
	_bFrontIn = bFrontIn;
	_carX = carX;
	_carY = carY;
	_caryaw = caryaw;
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

static int flag = 0;
static float k, b, dist;
static int flagt = 0;
float Actionpoint = 14.3;
bool tf = 0;

static void userDriverSetParam(bool *bFinished, float *cmdAcc, float *cmdBrake, float *cmdSteer, int *cmdGear)
{
	if (parameterSet == false)
	{
		PIDParamSetter();
	}
	else
	{
		startPoint = MAX(float(_speed * _speed * 0.00096), float(_speed * 0.715));
		circle c = getR(_midline[0][0], _midline[0][1], _midline[2][0], _midline[2][1], _midline[4][0], _midline[4][1]);

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
		if (flag == 5)
		{
			expectedSpeed = 100;
		}
		else
		{
			expectedSpeed = 85;
		}
		curSpeedErr = expectedSpeed - _speed;
		speedErrSum = 0.1 * speedErrSum + curSpeedErr;
		if (curSpeedErr > 0)
		{
			if (abs(*cmdSteer) < 0.1) //0.6
			{
				if (abs(_yawrate) > 0.2 && dataOutCount > 500)
				{
					*cmdAcc = constrain(0.0, 0.1, kp_s * curSpeedErr + ki_s * speedErrSum);
				}
				else
				{
					*cmdAcc = constrain(0, 0.3, kp_s * curSpeedErr + ki_s * speedErrSum);
				}
				*cmdBrake = 0;
			}
			else if (abs(*cmdSteer) < 0.40) 
			{
				if (abs(_yawrate) > 0.2 && dataOutCount > 500)
				{
					*cmdAcc = constrain(0, 0.2, kp_s * curSpeedErr + ki_s * speedErrSum); 
				}
				else
				{
					*cmdAcc = constrain(0, 0.2, kp_s * curSpeedErr + ki_s * speedErrSum);
				}
				*cmdBrake = 0;
			}
			else if (abs(*cmdSteer) > 0.70)
			{
				*cmdAcc = 0.11; 
				*cmdBrake = 0;
			}
			else
			{
				*cmdAcc = 0.1; 
				*cmdBrake = 0;
			}
		}
		else if (curSpeedErr < 0)
		{
			*cmdBrake = constrain(0.0, 0.7, -kp_s * curSpeedErr / 5);
			*cmdAcc = 0;
		}

		if (abs(_lotAngle) > (PI / 2 - 0.05) && abs(_lotAngle) < (PI / 2 + 0.05))
		{
			dist = abs(_carX - _lotX);
		}
		else
		{
			k = tan(_lotAngle);
			b = (_lotY - k * _lotX);
			dist = abs(k * _carX - _carY + b) / sqrt(k * k + 1);
		}
		distance = (_carX - _lotX) * (_carX - _lotX) + (_carY - _lotY) * (_carY - _lotY);
		double dis = (cos(_lotAngle) * (_carY - _lotY) - sin(_lotAngle) * (_carX - _lotX));
		err = ((_caryaw)-atan2(_carY - _lotY, _carX - _lotX));

		if (err > 3.14)
			err = -6.28 + err;
		else if (err < -3.14)
			err = 6.28 + err;
		err = err - dis / 20;
		float err2 = (_caryaw - _lotAngle);
		if (err2 > 3.14)
			err2 = -6.28 + err2;
		else if (err2 < -3.14)
			err2 = 6.28 + err2;
		errdipp = err - errtmp;
		errtmp = err;
		if (dotflag == 7)
		{
			*cmdAcc = 1;
			*cmdSteer = 0;
			*cmdGear = 1;
			*cmdBrake = 0;
		}
		else if (dotflag == 6)
		{
			cnt++;
			if (cnt > 3 && successflag == 1)
			{
				*cmdSteer = _yaw;
				*cmdGear = 1;
				*cmdAcc = constrain(0.4, 0.8, (cnt - cnttmp - 30) / 100);
				*cmdBrake = 0;
				float D;

				if (_caryaw * _lotAngle < 0)
				{
					D = -abs(_caryaw) - abs(_lotAngle) + 6.29;
				}
				else
					D = abs(_caryaw - _lotAngle);
				if (D > 1)
				{
					dotflag = 7;
				}
			}
			else
			{
				*cmdSteer = err2 * 2;
				*cmdAcc = 0;
				*cmdBrake = 0.2;
				*cmdGear = 0;
				if ((abs(distance - dis * dis) < 0.01 * 0.01) || tf == 1 || _speed < 0.005)
				{
					*cmdBrake = 1;
					tf = 1;
					if (abs(_speed) < 0.2)
					{
						*bFinished = 1;
						successflag = 1;
						cnttmp = cnt;
					}
					else
					{
						*cmdBrake = 1;
					}
				}
			}
		}

		else if (dotflag == 5)
		{
			*cmdBrake = 0.11;
			*cmdGear = -1;
			*cmdAcc = 0;
			*cmdSteer = err2 * 6;

			if (distance - dis * dis < 0.2 * 0.2)
			{
				dotflag = 6;
				cnt = 0;
			}
		}
		else if (dotflag == 4)
		{
			*cmdSteer = 20 * err2;
			*cmdGear = -1;
			*cmdAcc = 0.05;
			if (abs(_speed) > 8)
			{
				*cmdBrake = 0.6;
			}
			if ((distance - dis * dis < 0.4 * 0.4))
			{
				dotflag = 5;
			}
		}
		else if (dotflag == 8)
		{
			*cmdBrake = 0;
			*cmdGear = 1;
			*cmdAcc = 0.5;
			if (_speed < 0)
			{
				*cmdBrake = 1;
				*cmdAcc = 0;
			}
			*cmdSteer = -dis / 3;
			if (distance > 8 * 8)
			{
				dotflag = 3;
			}
		}
		else if (dotflag == 3)
		{
			if (distance - dis * dis < 3.5 * 3.5)
			{
				dotflag = 4;
				cnt = 0;
				if (abs(dis) > 0.6)
				{
					dotflag = 8;
				}
			}
			cnt = cnt + 1;
			*cmdBrake = 0;
			if (_speed > 0)
			{
				*cmdBrake = 1;
			}
			*cmdGear = -1;
			*cmdAcc = 0.25;
			kp = 20;
			ki = 10;
			if (abs(_speed) > 20)
			{
				*cmdBrake = 0.2;
				*cmdAcc = 0.05;
			}
			if (abs(err) > 0.5)
			{
				*cmdSteer = constrain(-1, 1, 2 * kp * err + ki * errdipp);
			}
			else
			{
				*cmdSteer = constrain(-1, 1, kp * err + ki * errdipp);
			}
			flag = 2; 
		}
		else if (dotflag == 2)
		{	
			cnt = cnt + 1;
			if (_speed < 0.05 || (abs(_lotAngle - _caryaw) < 0.01) || (abs(cos(_lotAngle) * (_carY - _lotY) - sin(_lotAngle) * (_carX - _lotX)) < 0.001) && dotflag < 3)
			{
				dotflag = 3;
				*cmdBrake = 0.05;
				cnt = 0;
			}
			if (abs(_lotAngle - _caryaw) < PI / 6) 
			{
				if (int(cnt) % 2 == 0)
				{
					*cmdBrake = 0.8;  
					*cmdSteer = -0.6; 
					*cmdAcc = 0;	  
				}
				else
				{
					*cmdBrake = 0.8;  
					*cmdSteer = -0.6; 
					*cmdAcc = 0;	  
				}
			}
			else
			{
				*cmdSteer = -1; 
				*cmdBrake = 0.15;
				*cmdAcc = 0;
			}

			cnt = cnt + 1;
		}
		else if (dotflag == 1) 
		{
			Actionpoint = constrain(12.2, 14.4, 12.3 + (c.r - 90) * 0.032);
			*cmdSteer = (_yaw - atan2(_midline[10][0] - _width / 3, _midline[10][1]));
			flag = 4;

			if (testflag == 0)
			{
				testflag = 1;
				if (abs(_lotX - _carX) > 20)
				{
					testflag = 1;
				}
				else
				{
					testflag = 2;
				}
			}
			if (abs(cos(_lotAngle) * (_carY - _lotY) - sin(_lotAngle) * (_carX - _lotX)) < Actionpoint && dotflag < 3)
			{
				dotflag = 2;
				cnt = 0;
			}
		}
		else if (dotflag == 0)
		{
			if (distance < 4000 && dotflag < 2 && successflag == 0)
			{
				dotflag = 1;
			}

			*cmdGear = 1;

			flag = 5;
			kp_d = 7.6; 
			ki_d = 0;	
			kd_d = 40;	
			int k0 = 50;
			D_err = _yaw - atan((k0 * (_midline[0][0])) / double(_speed + 1));

			D_errDiff = D_err - Tmp;
			Tmp = D_err;
			if (_speed > 20)
			{
				*cmdSteer = constrain(-1.0, 1.0, kp_d * D_err + ki_d * D_errSum + kd_d * D_errDiff);
			}
			else
			{
				*cmdAcc = 1;
			}
		}
		dataOutCount = dataOutCount + 1;
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
