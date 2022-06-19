
#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "headfile.h"

#define MT9V034_IMAGEH2  60u  	//行 HEIGHT 待采集摄像头图像高度行数
#define MT9V034_IMAGEW2  94u  	//列 WIDTH  待采集摄像头图像宽度列数

typedef uint8_t image_t[MT9V034_IMAGEH2][MT9V034_IMAGEW2];

typedef struct _pic
{
	uint16_t row[MT9V034_IMAGEH2];
	uint16_t diff[MT9V034_IMAGEH2];
	uint16_t diff5[MT9V034_IMAGEH2];
	uint16_t points;
	float meanx;
}pic_t;

uint8_t Atan2(float y, float x);
void Canny(image_t A, image_t B, uint8_t low_thres, uint8_t high_thres);
float fast_sqrtf(float x);
void dilate3(image_t input, image_t output);
void erode3(image_t input, image_t output);
void fast_gauss_conv3(image_t input, image_t output);
void fast_conv3(image_t input, image_t output);

void draw_road_line(image_t road, uint32_t len);
void dfs_save(uint8_t x, uint8_t y, uint8_t size);
uint32_t dfs_find(image_t edge, uint8_t x, uint8_t y, uint8_t len);
void find_road_lines(image_t edge, uint32_t line[][2], int32_t *len);
void road_judge(image_t binary);
void mid_lines_find(image_t binary);
void reset_dfs();
void road_judge(image_t binary);

#endif /* _IMAGE_H_ */


