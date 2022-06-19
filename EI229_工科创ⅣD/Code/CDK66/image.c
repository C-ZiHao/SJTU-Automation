
/**
 * @file    image.c
 * @brief   image
 * @data    2020/11/11
 */
#include <image.h>

#include "image.h"

static int dc_3[9] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
static int dr_3[9] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
static float k[3][3] = {{1/16, 2/16, 1/16}, {2/16, 4/16, 2/16}, {1/16, 2/16, 1/16}};

extern uint8_t speed;
extern float dir;

// convolution which is more friendly to cache
void fast_conv3(image_t input, image_t output){
    // convolution first line
    for(uint16_t r=1; r<MT9V034_IMAGEH2-1; r++){
		for(uint16_t c=1; c<MT9V034_IMAGEW2-1; c++){
			output[r][c] = input[r-1][c-1] * k[0][0];
			output[r][c] += input[r-1][c] * k[0][1];
			output[r][c] += input[r-1][c+1] * k[0][2];
        }
    }
    // convolution second line
    for(uint16_t r=1; r<MT9V034_IMAGEH2-1; r++){
		for(uint16_t c=1; c<MT9V034_IMAGEW2-1; c++){
			output[r][c] += input[r][c-1] * k[1][0];
			output[r][c] += input[r][c] * k[1][1];
			output[r][c] += input[r][c+1] * k[1][2];
        }
    }
    // convolution third line
    for(uint16_t r=1; r<MT9V034_IMAGEH2-1; r++){
		for(uint16_t c=1; c<MT9V034_IMAGEW2-1; c++){
			output[r][c] += input[r+1][c-1] * k[2][0];
			output[r][c] += input[r+1][c] * k[2][1];
			output[r][c] += input[r+1][c+1] * k[2][2];
        }
    }
}

void fast_gauss_conv3(image_t input, image_t output){
    // convolution first line
    for(uint16_t r=1; r<MT9V034_IMAGEH2-1; r++){
		for(uint16_t c=1; c<MT9V034_IMAGEW2-1; c++){
			output[r][c] = input[r-1][c-1] * 4921 >> 16;
			output[r][c] += input[r-1][c] * 8113 >> 16;
			output[r][c] += input[r-1][c+1] * 4921 >> 16;
        }
    }
    // convolution second line
    for(uint16_t r=1; r<MT9V034_IMAGEH2-1; r++){
		for(uint16_t c=1; c<MT9V034_IMAGEW2-1; c++){
			output[r][c] += input[r][c-1] * 8113 >> 16;
			output[r][c] += input[r][c] * 13382 >> 16;
			output[r][c] += input[r][c+1] * 8113 >> 16;
        }
    }
    // convolution third line
    for(uint16_t r=1; r<MT9V034_IMAGEH2-1; r++){
		for(uint16_t c=1; c<MT9V034_IMAGEW2-1; c++){
			output[r][c] += input[r+1][c-1] * 4921 >> 16;
			output[r][c] += input[r+1][c] * 8113 >> 16;
			output[r][c] += input[r+1][c+1] * 4921 >> 16;
        }
    }
}

void erode3(image_t input, image_t output){
	uint8_t tmp0=0,flag=0;
	for(uint16_t r=1; r<MT9V034_IMAGEH2-1; r++){
		for(uint16_t c=1; c<MT9V034_IMAGEW2-1; c++){
			output[r][c] = 0;
			flag=0;
			for(uint16_t i=0; i<9; i++)
			{
				tmp0 = input[r+dr_3[i]][c+dc_3[i]];
				if(tmp0==1) flag++;
			}
			if(flag>6)
			{
				output[r][c] = 1;
			}
		}
	}
}

void dilate3(image_t input, image_t output){
	uint8_t tmp0,flag=0;
	for(uint16_t r=1; r<MT9V034_IMAGEH2-1; r++){
		for(uint16_t c=1; c<MT9V034_IMAGEW2-1; c++){
			output[r][c] = 1;
			flag=0;
			for(uint16_t i=0; i<9; i++){
				tmp0 = input[r+dr_3[i]][c+dc_3[i]];
				if(tmp0==0) flag++;
			}
			if(flag>5)
			{
				output[r][c] = 0;
			}
		}
	}
}
pic_t image;
extern uint16_t road_id;
//white 1
void road_judge(image_t binary)
{
	uint16_t last_col=0,last_col5,mean_count;
	mean_count=0;
	image.meanx=0;
	uint32_t tmp_line[MT9V034_IMAGEH2];
	for(uint16_t h=3; h<MT9V034_IMAGEH2-3; h++)
	{
	  image.row[h]=0;
	  image.points=0;
	  image.diff[h]=100;
	  image.diff5[h]=100;

	  for(uint16_t w=3; w<MT9V034_IMAGEW2-3; w++)
	  {
		  if(binary[h][w]==0)
		  {

			  image.points++;
			  image.row[h]++;
			  tmp_line[h]= w ;
			  if(h>15 && h<45 && w>5 && w<90)
			  {
			     image.meanx=image.meanx+w;
			     mean_count++;
			  }
			  if(h>9)
			  {
			     image.diff[h]=w-last_col;
			     last_col=w;
			     if(h%5==0)
			      {
			    	 image.diff5[h]=w-last_col5;
			         last_col5=w;
			      }
			  }
		  }
	  }
	}

	image.meanx=image.meanx/mean_count;
	uint16_t most_num[100];
	uint16_t nor_num=0,max_tmp=0,diff_count=0;
	float mean_num=0,sum_num=0;
	for(uint16_t h=10; h<MT9V034_IMAGEH2-10; h++)
	{
		if(image.diff[h]<100)
		{
		  most_num[image.diff[h]]++;
		  most_num[image.diff[h]+1]++;
		  most_num[image.diff[h]-1]++;
		  diff_count++;
		  sum_num+=image.diff[h];
		}
	}
	for(uint16_t i=0; i<100; i++)
	{
		if(most_num[i]>max_tmp) {nor_num=i;max_tmp=most_num[i]/3;}
	}

	mean_num=sum_num/diff_count;

	uint16_t zebra_count=0,cross_count=0,zebra_flag=0,cross_flag=0,straight_count=0,curve_count=0;
	uint16_t start_flag=0,start_count=0,stra2_count=0;
	for(uint16_t h=3; h<MT9V034_IMAGEH2-3; h++)
	{
		if(image.row[h]>35 && h>21)      {start_count++;}
		else if(image.row[h]<15) {start_count=0;}

		if(image.row[h]>60)      {zebra_count++;}
		else if(image.row[h]<40) {zebra_count=0;}

		if(image.row[h]>70)      {cross_count++;}
		else if(image.row[h]<40) {cross_count=0;}

		if(zebra_count>10) zebra_flag=1;
		if(cross_count>3) cross_flag=1;
		if(start_count>3) start_flag=1;

	    if(abs(image.diff[h])<3)     {straight_count++;}
	    if(image.row[h]>30 && h<24)  {curve_count++;}
	    for(uint16_t w=5; w<MT9V034_IMAGEW2-5; w++)
	    {
	       if(binary[h][w]==0 && abs(w-image.meanx)<3) {stra2_count++;break;}
	    }
	}

	//printf("%d  ,  %f\n",stra2_count,image.meanx);
	if(zebra_flag==1)
	{
		road_id=6;
		speed=0;
		dir=0;
		BOARD_I2C_GPIO(1<<(int)(8-dir*8));
	}
	else if(cross_flag==1)
	{
		road_id=4;
	    speed=2;
	    dir=0;
	    BOARD_I2C_GPIO(1<<(int)(8-dir*8));
	}
	else if(start_flag==1)
	{
		road_id=5;
	    speed=4;
	    dir=0;
	    BOARD_I2C_GPIO(1<<(int)(8-dir*8));
	}
	else if(curve_count>4 && straight_count>MT9V034_IMAGEH2/3)
	{
		road_id=3;
	    dir= -(float)(tmp_line[15]-MT9V034_IMAGEW2/2)/MT9V034_IMAGEW2;
	    speed = MINMAX(1,5-4*abs(dir),5);
	    dir= MIN(MAX(dir, -0.99), 0.99);
	    BOARD_I2C_GPIO(1<<(int)(8-dir*8));
	}
	else if(stra2_count>MT9V034_IMAGEH2*2/3)
	{
		road_id=1;
	    dir = 0;
	    speed =5;
	    BOARD_I2C_GPIO(1<<(int)(8-dir*8));
	}
	else if(straight_count>MT9V034_IMAGEH2/2)
	{
		road_id=2;
		dir = -(float)(tmp_line[15]-MT9V034_IMAGEW2/2)/MT9V034_IMAGEW2;
		dir= MIN(MAX(dir, -0.99), 0.99);
	    speed =5;
	    BOARD_I2C_GPIO(1<<(int)(8-dir*8));
	}
	else
	{
		LED1_TOGGLE();
	    road_id=0;
	    speed = 6;
	    dir = -1;
	    BOARD_I2C_GPIO((1<<16)-1);
	}
	LED1_flash(121- speed * 20);


}



float fast_sqrtf(float x)
{
  float xhalf = 0.5f * x;
  int i = *(int*)&x;
  i  = 0x5f375a86 - ( i >> 1 );
  x = *(float*)&i;
  x = x*(1.5f-(xhalf*x*x));
  return 1/x;
}

void Canny(image_t A, image_t B, uint8_t low_thres, uint8_t high_thres){
	static image_t CannyAm, Sector;
	int Xg = 0; //X方向梯度
	int Yg = 0; //Y方向梯度
	uint8_t loss = 2;
	uint8_t temp;
	for (int i = 0; i < MT9V034_IMAGEH2; i++)
	{
		for (int j = 0; j < MT9V034_IMAGEW2; j++)
		{
//			Xg=(A[i][j]+A[i+1][j]-A[i+1][j+1]-A[i][j+1])>>1;
//			Yg=(-A[i][j]+A[i+1][j]-A[i][j+1]+A[i+1][j+1])>>1;
			// sobel kernel in the horizontal direction
            Xg  = ((int)A [i-1][j+1]
                - (int)A [i-1][j]
                + ((int)A[i+0][j+1]<<1)
                - ((int)A[i+0][j]<<1)
                + (int)A [i+1][j+1]
                - (int)A [i+1][j]) >> 1;

            // sobel kernel in the vertical direction
            Yg  = ((int)A [i][j-1]
                + ((int)A[i][j+0]<<1)
                + (int)A [i][j+1]
                - (int)A [i+1][j-1]
                - ((int)A[i+1][j+0]<<1)
                - (int)A [i+1][j+1]) >> 1;
			CannyAm[i][j] = sqrt(Xg*Xg+Yg*Yg); //求幅值，快速开平方算法
			Sector[i][j] = Atan2(Yg, Xg);  //求梯度方向分区
		}
	}
	//非极大抑制
	for(int y=1; y<MT9V034_IMAGEH2-1; y++)
	{
		for(int x=1; x<MT9V034_IMAGEW2-1; x++)
		{
			switch(Sector[y][x]){

			case 0: //水平方向
				if( CannyAm[y][x] < (CannyAm[y][x+1] - loss) || CannyAm[y][x] < (CannyAm[y][x-1] - loss) )
				{
					CannyAm[y][x] = 0;
				}
				break; // end of 0
			case 1: //右上、左下
				if( CannyAm[y][x] < (CannyAm[y-1][x+1] - loss) || CannyAm[y][x] < (CannyAm[y+1][x-1] - loss) )
				{
					CannyAm[y][x] = 0;
				}
				break; //end of 1
			case 2: //竖直方向
				if( CannyAm[y][x] < (CannyAm[y-1][x] - loss) || CannyAm[y][x] < (CannyAm[y+1][x] - loss) )
				{
					CannyAm[y][x] = 0;
				}
				break; //end of 2
			case 3: //左上、右下
				if( CannyAm[y][x] < (CannyAm[y-1][x-1] - loss) || CannyAm[y][x] < (CannyAm[y+1][x+1] - loss) )
				{
					CannyAm[y][x] = 0;
				}
				break; //end of 3
			}
		} //end of for(x)
	} // end of for(y)


	for(int y=1; y<MT9V034_IMAGEH2-1; y++)
	{
		for(int x=1; x<MT9V034_IMAGEW2-1; x++)
		{
			if( CannyAm[y][x] < low_thres ) //低于低阈值
			{
				B[y][x] = 255;
			}
			else if( CannyAm[y][x] > high_thres ) //高于高阈值
			{
				B[y][x] = 0;
			}
			else
			{
				temp = A[y+1][x-1];
				if(temp < A[y+1][x]) temp = A[y+1][x];
				if(temp < A[y+1][x+1]) temp = A[y+1][x+1];
				if(temp < A[y][x-1]) temp = A[y][x-1];
				if(temp < A[y][x]) temp =A[y][x];
				if(temp < A[y][x+1]) temp = A[y][x+1];
				if(temp < A[y-1][x-1]) temp = A[y-1][x-1];
				if(temp < A[y-1][x]) temp = A[y-1][x];
				if(temp < A[y-1][x+1]) temp = A[y-1][x+1];

				if(temp > high_thres)
				{
					B[y][x] = 0;
					CannyAm[y][x] = 0;
				} //end of if
				else
				{
					B[y][x] = 255;
					CannyAm[y][x] = 255;
				}
			}
		}
	}
}


//=============反正切函数============LQ=
/********************************************
*  直接计算几个分界点的值,只需判断 y/x 的值所
* 对应的角度在哪个范围即可，不用实时求atan
*--------------------------------------------
*    角度范围        |         弧度范围
*--------------------------------------------
* 0     ~ 22.5  ------> 0         ~ 0.3926990
* 22.5  ~ 67.5  ------> 0.3926990 ~ 1.1780972
* 67.5  ~ 112.5 ------> 1.1780972 ~ 1.9634954
* 112.5 ~ 157.5 ------> 1.9634954 ~ 2.7488935
* 157.5 ~ 180   ------> 2.7488935 ~ 3.1415926
*--------------------------------------------
*         y/x值对应弧度
*  0          ----  0.41421356  水平方向
*  0.41421347 ----  2.41421356  右上、左下
*  2.41421326 ---- -2.41421356  竖直方向
* -2.41421362 ---- -0.41421356  左上、右下
* -0.41421365 ----  0           水平方向
********************************************/
uint8_t Atan2(float y, float x)
{
    float tanNum;
    uint8_t alpha; //返回角度
    tanNum = y/x;
    if( tanNum> -0.41421365 && tanNum< 0.41421356 )
    {
       alpha = 0; //水平方向
    }
    else if( tanNum>= 0.41421356 && tanNum< 2.41421356)
    {
       alpha = 1; //右上、左下
    }
    else if( tanNum<= -0.41421356 && tanNum> -2.41421362)
    {
       alpha = 3; //左上、右下
    }
    else
    {
       alpha = 2; //竖直方向
     }
    return alpha; //方向
} //end of function Atan2


const int dx[] = {-1,  0,  1, -1, 1,};
const int dy[] = {-1, -1, -1,  0, 0,};
const uint32_t d_size = sizeof(dx) / sizeof(dx[0]);
#define ROAD_LINE_LENGTH_MIN    (30)
uint16_t length[MT9V034_IMAGEH2][MT9V034_IMAGEW2];

void reset_dfs(){
    memset(length, 0, sizeof(length));
}

uint32_t dfs_find(image_t edge, uint8_t x, uint8_t y, uint8_t len){

    uint8_t _x, _y, remain=0, temp;
    len++;
    length[y][x] = len;
    for(uint16_t i=0; i<d_size; i++){
        _x = x + dx[i];
        _y = y + dy[i];
        if(0<=_x && _x<MT9V034_IMAGEW2 && 0<=_y && _y<MT9V034_IMAGEH2 && edge[_y][_x]==0 && length[_y][_x]==0){
            temp = dfs_find(edge, _x, _y, len);
            if(temp > remain) remain = temp;
        }
    }
    length[y][x] = len + remain;
    return remain + 1;
}

extern uint32_t line[200][2];
void dfs_save(uint8_t x, uint8_t y, uint8_t size){
	uint8_t len = length[y][x], _x, _y;
    length[y][x] = 0;
    size--;
    if(size==1) return;
    line[size][0] = x;
    line[size][1] = y;
   // printf("%d,%d\n",x,y);
    //printf("%d,%d\n",line[size][0],line[size][1]);
    for(uint32_t i=0; i<d_size; i++){
        _x = x + dx[i];
        _y = y + dy[i];
        if(0<_x && _x<MT9V034_IMAGEW2 && 0<_y && _y<MT9V034_IMAGEH2 && length[_y][_x]==len && size>0){
            dfs_save( _x, _y, size);
        }
    }
}


void find_road_lines(image_t edge, uint32_t line[][2], int32_t *len){
    uint32_t _len=0;
    reset_dfs();
    for(uint16_t y=MT9V034_IMAGEH2 - 10; y>=10; y--){
        for(uint16_t x=MT9V034_IMAGEW2-10; x>=10; x--){
            _len = dfs_find(edge, x, y, 0);
            if(_len > ROAD_LINE_LENGTH_MIN){
                if(_len < *len) *len = _len;
                dfs_save(x, y, *len);
                goto found;
            }
        }
    }
    *len = 0;
  found:
       return;
}

void draw_road_line(image_t road,uint32_t len){
    memset(road, 1, sizeof(image_t));
    for(uint32_t i=0; i<len; i++)
    {
        if(5<line[i][1] && line[i][1]<MT9V034_IMAGEH2-5 && 5<line[i][0] && line[i][0]<MT9V034_IMAGEW2-5)
        {
        	road[line[i][1]][line[i][0]] = 0;

        }
    }
}


