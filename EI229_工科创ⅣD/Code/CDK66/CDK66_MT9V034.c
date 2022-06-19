/*
 * CDK66_MT9V034.c
 *
 * Created on: 2020年10月7日
 *     Author: Bing Wang
 */
#include "CDK66_MT9V034.h"

/* 对比度 高对比度0x03c7  低对比度0x01c7  注意 高对比度会使图像变暗 */
//#define CAMERA_CONTRAST			0x01c7
#define CAMERA_CONTRAST				0x03c7

/* 自动曝光开关 默认打开  设置为0 关闭自动曝光 */
#define CAMERA_AUTO_EXPOSURE		1

/* 自动曝光模式下 亮度 调节范围 1-64 */
#define CAMERA_AUTO_EXPOSURE_BRIGHTNESS 30

/* 可以在自动曝光的情况下设置固定帧率 */
/* 自动曝光模式需要设置曝光时间上限 调节范围 1–32765 */
/* 注意 当帧率过高时 如果设置的曝光时间过长 帧率可能会自适应下调 */
/* 例如使用100帧时 最大曝光时间超过317 会导致帧率下降 */
/* 曝光时间越长 图像越亮 */
#define CAMERA_MAX_EXPOSURE_TIME	250
#define CAMERA_MIN_EXPOSURE_TIME	1

gpio_pin_config_t GPO_config = {
	.pinDirection = kGPIO_DigitalOutput,
	.outputLogic = 1U
};

gpio_pin_config_t GPI_config = {
    .pinDirection = kGPIO_DigitalInput,
    .outputLogic = 0U
};

/* 非自动曝光模式下 可以调节曝光时间来调节图像整体亮度 调节范围 0–32765 */
/* 注意 当帧率过高时 如果设置的曝光时间过长 帧率可能会自适应下调 */
/* 曝光时间越长 图像越亮 */
#define CAMERA_EXPOSURE_TIME		150

/*************************************************************************
* 函数名称: MT9V034_Init(uint8_t fps)
* 功能说明: 摄像头初始化
* 参数说明: fps  帧率
* 函数返回: 无
* 修改时间: 2020年10月08日
* 备    注:
*************************************************************************/
void MT9V034_Init(uint8_t fps)
{
	uint16_t data = 0;

	//摄像头SCCB接口初始化
	SCCB_Init();

	if(SCCB_RegRead(MT9V034_I2C_ADDR>>1,MT9V034_CHIP_VERSION,&data) == 0)  //读取摄像头版本寄存器
	{
		if(data != MT9V034_CHIP_ID)                                        //芯片ID不正确，说明没有正确读取导数据，检查接线
		{
			OLED_P6x8Str(2,1,(uint8_t*)"V034 NG");
			while(1);
		}
		else                                                               //芯片ID正确
		{
			OLED_P6x8Str(2,1,(uint8_t*)"V034 OK");
		}
	}
	else
	{
		OLED_P6x8Str(2,1,(uint8_t*)"V034 NG");
		while(1);                                                         //摄像头识别失败，停止运行
	}
	/* 恢复默认配置 */
	MT9V034_Default_Settings();

    /* 设置摄像头图像4*4分频输出PCLK 27/4 = 6.75M ,BIT4,5镜像设置:上下左右均镜像 */
	MT9V034_SetFrameResolution(MT9V034_IMAGEH, MT9V034_IMAGEW);

    /* 设置帧率 */
    MT9V034_SetFrameRate(fps);

    /* 曝光设置 */
    MT9V034_SetAutoExposure(CAMERA_AUTO_EXPOSURE);

	SCCB_RegWrite(MT9V034_I2C_ADDR, 0x2C, 0x0004);  //参考电压设置   1.4v
    SCCB_RegWrite(MT9V034_I2C_ADDR, MT9V034_ANALOG_CTRL, MT9V034_ANTI_ECLIPSE_ENABLE);  //反向腐蚀
    SCCB_RegWrite(MT9V034_I2C_ADDR, MT9V034_ADC_RES_CTRL_REG, 0x0303);      //0x1C  here is the way to regulate darkness :)
    ////
    SCCB_RegWrite(MT9V034_I2C_ADDR,0x13,0x2D2E);//We also recommended using R0x13 = 0x2D2E with this setting for better column FPN.
    SCCB_RegWrite(MT9V034_I2C_ADDR,0x20,CAMERA_CONTRAST);//0x01C7对比度差，发白；0x03C7对比度提高 Recommended by design to improve performance in HDR mode and when frame rate is low.
    SCCB_RegWrite(MT9V034_I2C_ADDR,0x24,0x0010);//Corrects pixel negative dark offset when global reset in R0x20[9] is enabled.
    SCCB_RegWrite(MT9V034_I2C_ADDR,0x2B,0x0003);//Improves column FPN.
    SCCB_RegWrite(MT9V034_I2C_ADDR,0x2F,0x0003);//Improves FPN at near-saturation.

    SCCB_RegWrite(MT9V034_I2C_ADDR,MT9V034_V2_CTRL_REG_A, 0x001A);        //0x32   0x001A
    SCCB_RegWrite(MT9V034_I2C_ADDR,MT9V034_HDR_ENABLE_REG,0x0103);        //0x0F High Dynamic Range enable,bit is set (R0x0F[1]=1), the sensor uses black level correction values from one green plane, which are applied to all colors.

    /* 0xA5  图像亮度  60  1-64  */
    SCCB_RegWrite(MT9V034_I2C_ADDR,MT9V034_AGC_AEC_DESIRED_BIN_REG, CAMERA_AUTO_EXPOSURE_BRIGHTNESS);
    SCCB_RegWrite(MT9V034_I2C_ADDR, MT9V034_RESET, 0x03);          //0x0c  复位
}

/*************************************************************************
* 函数名称: MT9V034_SetFrameRate(uint8_t fps)
* 功能说明: 摄像头帧率设置
* 参数说明: fps  帧率
* 函数返回: 无
* 修改时间: 2020年10月08日
* 备注: 帧率其实是通过增加空白行实现的， 实测的帧率与虚拟行之间的关系如下
*    帧率		200		150		100		70.9	50		20		10
*    虚拟行	39		92		199		330		515		1450	3000
*************************************************************************/
void MT9V034_SetFrameRate(uint8_t fps)
{
    float vertical = 0;
    if(fps > 200)
    {
        vertical = 2;
    }
    else if(fps >= 150)
    {
        vertical = -1.06 * fps + 251;
    }
    else if(fps > 100)
    {
        vertical = -2.14 * fps + 413;
    }
    else if(fps >= 71)
    {
        vertical = -4.5017 * fps + 649.17;
    }
    else if(fps > 50)
    {
        vertical = -8.8517 * fps + 957.58;
    }
    else if(fps > 20)
    {
        vertical = -31.167 * fps + 2073.3;
    }
    else
    {
        vertical = -155 * fps + 4550;
    }
    /* 虚拟行 设置帧率时间 2–32288 */
    SCCB_RegWrite(MT9V034_I2C_ADDR, MT9V034_VERTICAL_BLANKING, (uint16_t)vertical);
}

/*************************************************************************
* 函数名称: MT9V034_SetFrameResolution(uint16_t height, uint16_t width)
* 功能说明: 摄像头窗口大小设置和图像翻转设置
* 参数说明: height  图像高度
* 参数说明: width   图像宽度
* 函数返回: 无
* 修改时间: 2020年10月08日
* 备注:
*************************************************************************/
static void MT9V034_SetFrameResolution(uint16_t height, uint16_t width)
{
    uint16_t data = 0;

    if((height*4) <= MAX_IMAGE_HEIGHT)   //判断行是否4分频
    {
        height *= 4;
        data |= MT9V034_READ_MODE_ROW_BIN_4;
    }
    else if((height*2)<=MAX_IMAGE_HEIGHT )  //判断行是否2分频
    {
        height *= 2;
        data |= MT9V034_READ_MODE_ROW_BIN_2;
    }

    if((width*4)<=MAX_IMAGE_WIDTH )   //判断列是否4分频
    {
        width *= 4;
        data |= MT9V034_READ_MODE_COL_BIN_4;
    }
    else if((width*2)<=MAX_IMAGE_WIDTH )   //判断列是否2分频
    {
        width *= 2;
        data |= MT9V034_READ_MODE_COL_BIN_2;
    }

    //         水平翻转                     垂直翻转
    data |= (MT9V034_READ_MODE_ROW_FLIP|MT9V034_READ_MODE_COLUMN_FLIP);  //需要翻转的可以打开注释
    MTV_IICWriteReg16(MT9V034_READ_MODE, data);       //写寄存器，配置行分频

    MTV_IICWriteReg16(MT9V034_WINDOW_WIDTH,  width);  //读取图像的列数  改变此处也可改变图像输出大小，不过会丢失视角
    MTV_IICWriteReg16(MT9V034_WINDOW_HEIGHT, height); //读取图像的行数  改变此处也可改变图像输出大小，不过会丢失视角

    MTV_IICWriteReg16(MT9V034_COLUMN_START, MT9V034_COLUMN_START_MIN);  //列开始
    MTV_IICWriteReg16(MT9V034_ROW_START, MT9V034_ROW_START_MIN);        //行开始
}

/*************************************************************************
* 函数名称: MT9V034_SetAutoExposure(uint8_t enable)
* 功能说明: 自动曝光设置
* 参数说明: enable    0: 关闭自动曝光
* 函数返回: 无
* 修改时间: 2020年10月08日
* 备    注:
*************************************************************************/
void MT9V034_SetAutoExposure(uint8_t enable)
{
    uint16_t reg = 0;

    SCCB_RegRead(MT9V034_I2C_ADDR, MT9V034_AEC_AGC_ENABLE,&reg);
    if(enable)
    {
        /* 开启自动曝光自动增益 */
        SCCB_RegWrite(MT9V034_I2C_ADDR, MT9V034_AEC_AGC_ENABLE, reg|MT9V034_AEC_ENABLE|MT9V034_AGC_ENABLE);
        /* 最大曝光时间 修改这里可以修改比较暗时的图像整体亮度*/
        SCCB_RegWrite(MT9V034_I2C_ADDR, MT9V034_MAX_EXPOSURE_REG, CAMERA_MAX_EXPOSURE_TIME);
        /* 最小曝光时间 修改这里可以修改遇到强光时的图像整体亮度*/
        SCCB_RegWrite(MT9V034_I2C_ADDR, MT9V034_MIN_EXPOSURE_REG, CAMERA_MIN_EXPOSURE_TIME);
        /* 最大增益 增大这里 图像偏暗的情况下保留更多的细节 但是可能产生噪点 0-60*/
        SCCB_RegWrite(MT9V034_I2C_ADDR, MT9V034_MAX_GAIN_REG, 20);
        /* 0xB0  用于AEC/AGC直方图像素数目,22560 最大44000  */
        SCCB_RegWrite(MT9V034_I2C_ADDR, MT9V034_AGC_AEC_PIXEL_COUNT_REG, 22560);
    }
    else
    {
        /* 关闭自动曝光 */
        SCCB_RegWrite(MT9V034_I2C_ADDR, MT9V034_AEC_AGC_ENABLE, reg&~(MT9V034_AEC_ENABLE|MT9V034_AGC_ENABLE));

        /* 0xAB  最大模拟增益     64 */
        SCCB_RegWrite(MT9V034_I2C_ADDR, MT9V034_MAX_GAIN_REG, 30);

        /* 0x0B 设置曝光时间 0–32765 */
        SCCB_RegWrite(MT9V034_I2C_ADDR,MT9V034_TOTAL_SHUTTER_WIDTH,CAMERA_EXPOSURE_TIME);
    }
}

/*************************************************************************
* 函数名称: MT9V034_Default_Settings(void)
* 功能说明: 摄像头默认配置初始化
* 参数说明: 无
* 函数返回: 无
* 修改时间: 2020年10月08日
* 备    注:
*************************************************************************/
void MT9V034_Default_Settings(void)
{
    MTV_IICWriteReg16(0x01, 0x0001);   //COL_WINDOW_START_CONTEXTA_REG
    MTV_IICWriteReg16(0x02, 0x0004);   //ROW_WINDOW_START_CONTEXTA_REG
    MTV_IICWriteReg16(0x03, 0x01E0);   //ROW_WINDOW_SIZE_CONTEXTA_REG
    MTV_IICWriteReg16(0x04, 0x02F0);   //COL_WINDOW_SIZE_CONTEXTA_REG
    MTV_IICWriteReg16(0x05, 0x005E);   //HORZ_BLANK_CONTEXTA_REG
    MTV_IICWriteReg16(0x06, 0x002D);   //VERT_BLANK_CONTEXTA_REG
    MTV_IICWriteReg16(0x07, 0x0188);   //CONTROL_MODE_REG
    MTV_IICWriteReg16(0x08, 0x01BB);   //COARSE_SHUTTER_WIDTH_1_CONTEXTA
    MTV_IICWriteReg16(0x09, 0x01D9);   //COARSE_SHUTTER_WIDTH_2_CONTEXTA
    MTV_IICWriteReg16(0x0A, 0x0164);   //SHUTTER_WIDTH_CONTROL_CONTEXTA
    MTV_IICWriteReg16(0x0B, 0x0000);   //COARSE_SHUTTER_WIDTH_TOTAL_CONTEXTA
    MTV_IICWriteReg16(0x0C, 0x0000);   //RESET_REG
    MTV_IICWriteReg16(0x0D, 0x0300);   //READ_MODE_REG
    MTV_IICWriteReg16(0x0E, 0x0000);   //READ_MODE2_REG
    MTV_IICWriteReg16(0x0F, 0x0000);   //PIXEL_OPERATION_MODE
    MTV_IICWriteReg16(0x10, 0x0040);   //RAMP_START_DELAY
    MTV_IICWriteReg16(0x11, 0x8042);   //OFFSET_CONTROL
    MTV_IICWriteReg16(0x12, 0x0022);   //AMP_RESET_BAR_CONTROL
    MTV_IICWriteReg16(0x13, 0x2D2E);   //5T_PIXEL_RESET_CONTROL
    MTV_IICWriteReg16(0x14, 0x0E02);   //4T_PIXEL_RESET_CONTROL
    MTV_IICWriteReg16(0x15, 0x0E32);   //TX_CONTROL
    MTV_IICWriteReg16(0x16, 0x2802);   //5T_PIXEL_SHS_CONTROL
    MTV_IICWriteReg16(0x17, 0x3E38);   //4T_PIXEL_SHS_CONTROL
    MTV_IICWriteReg16(0x18, 0x3E38);   //5T_PIXEL_SHR_CONTROL
    MTV_IICWriteReg16(0x19, 0x2802);   //4T_PIXEL_SHR_CONTROL
    MTV_IICWriteReg16(0x1A, 0x0428);   //COMPARATOR_RESET_CONTROL
    MTV_IICWriteReg16(0x1B, 0x0000);   //LED_OUT_CONTROL
    MTV_IICWriteReg16(0x1C, 0x0302);   //DATA_COMPRESSION
    MTV_IICWriteReg16(0x1D, 0x0040);   //ANALOG_TEST_CONTROL
    MTV_IICWriteReg16(0x1E, 0x0000);   //SRAM_TEST_DATA_ODD
    MTV_IICWriteReg16(0x1F, 0x0000);   //SRAM_TEST_DATA_EVEN
    MTV_IICWriteReg16(0x20, 0x03C7);   //BOOST_ROW_EN
    MTV_IICWriteReg16(0x21, 0x0020);   //I_VLN_CONTROL
    MTV_IICWriteReg16(0x22, 0x0020);   //I_VLN_AMP_CONTROL
    MTV_IICWriteReg16(0x23, 0x0010);   //I_VLN_CMP_CONTROL
    MTV_IICWriteReg16(0x24, 0x001B);   //I_OFFSET_CONTROL
//    MTV_IICWriteReg1G=0x25, 0x0000); // I_BANDGAP_CONTROL - TRIMMED PER DIE
    MTV_IICWriteReg16(0x26, 0x0004);   //I_VLN_VREF_ADC_CONTROL
    MTV_IICWriteReg16(0x27, 0x000C);   //I_VLN_STEP_CONTROL
    MTV_IICWriteReg16(0x28, 0x0010);   //I_VLN_BUF_CONTROL
    MTV_IICWriteReg16(0x29, 0x0010);   //I_MASTER_CONTROL
    MTV_IICWriteReg16(0x2A, 0x0020);   //I_VLN_AMP_60MHZ_CONTROL
    MTV_IICWriteReg16(0x2B, 0x0004);   //VREF_AMP_CONTROL
    MTV_IICWriteReg16(0x2C, 0x0004);   //VREF_ADC_CONTROL
    MTV_IICWriteReg16(0x2D, 0x0004);   //VBOOST_CONTROL
    MTV_IICWriteReg16(0x2E, 0x0007);   //V_HI_CONTROL
    MTV_IICWriteReg16(0x2F, 0x0003);   //V_LO_CONTROL
    MTV_IICWriteReg16(0x30, 0x0003);   //V_AMP_CAS_CONTROL
    MTV_IICWriteReg16(0x31, 0x0027);   //V1_CONTROL_CONTEXTA
    MTV_IICWriteReg16(0x32, 0x001A);   //V2_CONTROL_CONTEXTA
    MTV_IICWriteReg16(0x33, 0x0005);   //V3_CONTROL_CONTEXTA
    MTV_IICWriteReg16(0x34, 0x0003);   //V4_CONTROL_CONTEXTA
    MTV_IICWriteReg16(0x35, 0x0010);   //GLOBAL_GAIN_CONTEXTA_REG
    MTV_IICWriteReg16(0x36, 0x8010);   //GLOBAL_GAIN_CONTEXTB_REG
    MTV_IICWriteReg16(0x37, 0x0000);   //VOLTAGE_CONTROL
    MTV_IICWriteReg16(0x38, 0x0000);   //IDAC_VOLTAGE_MONITOR
    MTV_IICWriteReg16(0x39, 0x0027);   //V1_CONTROL_CONTEXTB
    MTV_IICWriteReg16(0x3A, 0x0026);   //V2_CONTROL_CONTEXTB
    MTV_IICWriteReg16(0x3B, 0x0005);   //V3_CONTROL_CONTEXTB
    MTV_IICWriteReg16(0x3C, 0x0003);   //V4_CONTROL_CONTEXTB
    MTV_IICWriteReg16(0x40, 0x0080);   //DARK_AVG_THRESHOLDS
    MTV_IICWriteReg16(0x46, 0x231D);   //CALIB_CONTROL_REG (AUTO)
    MTV_IICWriteReg16(0x47, 0x0080);   //STEP_SIZE_AVG_MODE
    MTV_IICWriteReg16(0x48, 0x0020);   //ROW_NOISE_CONTROL
    MTV_IICWriteReg16(0x4C, 0x0002);   //NOISE_CONSTANT
    MTV_IICWriteReg16(0x60, 0x0000);   //PIXCLK_CONTROL
    MTV_IICWriteReg16(0x67, 0x0000);   //TEST_DATA
    MTV_IICWriteReg16(0x6C, 0x0000);   //TILE_X0_Y0
    MTV_IICWriteReg16(0x70, 0x0000);   //TILE_X1_Y0
    MTV_IICWriteReg16(0x71, 0x002A);   //TILE_X2_Y0
    MTV_IICWriteReg16(0x72, 0x0000);   //TILE_X3_Y0
    MTV_IICWriteReg16(0x7F, 0x0000);   //TILE_X4_Y0
    MTV_IICWriteReg16(0x99, 0x0000);   //TILE_X0_Y1
    MTV_IICWriteReg16(0x9A, 0x0096);   //TILE_X1_Y1
    MTV_IICWriteReg16(0x9B, 0x012C);   //TILE_X2_Y1
    MTV_IICWriteReg16(0x9C, 0x01C2);   //TILE_X3_Y1
    MTV_IICWriteReg16(0x9D, 0x0258);   //TILE_X4_Y1
    MTV_IICWriteReg16(0x9E, 0x02F0);   //TILE_X0_Y2
    MTV_IICWriteReg16(0x9F, 0x0000);   //TILE_X1_Y2
    MTV_IICWriteReg16(0xA0, 0x0060);   //TILE_X2_Y2
    MTV_IICWriteReg16(0xA1, 0x00C0);   //TILE_X3_Y2
    MTV_IICWriteReg16(0xA2, 0x0120);   //TILE_X4_Y2
    MTV_IICWriteReg16(0xA3, 0x0180);   //TILE_X0_Y3
    MTV_IICWriteReg16(0xA4, 0x01E0);   //TILE_X1_Y3
    MTV_IICWriteReg16(0xA5, 0x003A);   //TILE_X2_Y3
    MTV_IICWriteReg16(0xA6, 0x0002);   //TILE_X3_Y3
    MTV_IICWriteReg16(0xA8, 0x0000);   //TILE_X4_Y3
    MTV_IICWriteReg16(0xA9, 0x0002);   //TILE_X0_Y4
    MTV_IICWriteReg16(0xAA, 0x0002);   //TILE_X1_Y4
    MTV_IICWriteReg16(0xAB, 0x0040);   //TILE_X2_Y4
    MTV_IICWriteReg16(0xAC, 0x0001);   //TILE_X3_Y4
    MTV_IICWriteReg16(0xAD, 0x01E0);   //TILE_X4_Y4
    MTV_IICWriteReg16(0xAE, 0x0014);   //X0_SLASH5
    MTV_IICWriteReg16(0xAF, 0x0000);   //X1_SLASH5
    MTV_IICWriteReg16(0xB0, 0xABE0);   //X2_SLASH5
    MTV_IICWriteReg16(0xB1, 0x0002);   //X3_SLASH5
    MTV_IICWriteReg16(0xB2, 0x0010);   //X4_SLASH5
    MTV_IICWriteReg16(0xB3, 0x0010);   //X5_SLASH5
    MTV_IICWriteReg16(0xB4, 0x0000);   //Y0_SLASH5
    MTV_IICWriteReg16(0xB5, 0x0000);   //Y1_SLASH5
    MTV_IICWriteReg16(0xB6, 0x0000);   //Y2_SLASH5
    MTV_IICWriteReg16(0xB7, 0x0000);   //Y3_SLASH5
    MTV_IICWriteReg16(0xBF, 0x0016);   //Y4_SLASH5
    MTV_IICWriteReg16(0xC0, 0x000A);   //Y5_SLASH5
    MTV_IICWriteReg16(0xC2, 0x18D0);   //DESIRED_BIN
    MTV_IICWriteReg16(0xC3, 0x007F);   //EXP_SKIP_FRM_H
    MTV_IICWriteReg16(0xC4, 0x007F);   //EXP_LPF
    MTV_IICWriteReg16(0xC5, 0x007F);   //GAIN_SKIP_FRM
    MTV_IICWriteReg16(0xC6, 0x0000);   //GAIN_LPF_H
    MTV_IICWriteReg16(0xC7, 0x4416);   //MAX_GAIN
    MTV_IICWriteReg16(0xC8, 0x4421);   //MIN_COARSE_EXPOSURE
    MTV_IICWriteReg16(0xC9, 0x0001);   //MAX_COARSE_EXPOSURE
    MTV_IICWriteReg16(0xCA, 0x0004);   //BIN_DIFF_THRESHOLD
    MTV_IICWriteReg16(0xCB, 0x01E0);   //AUTO_BLOCK_CONTROL
    MTV_IICWriteReg16(0xCC, 0x02F0);   //PIXEL_COUNT
    MTV_IICWriteReg16(0xCD, 0x005E);   //LVDS_MASTER_CONTROL
    MTV_IICWriteReg16(0xCE, 0x002D);   //LVDS_SHFT_CLK_CONTROL
    MTV_IICWriteReg16(0xCF, 0x01DE);   //LVDS_DATA_CONTROL
    MTV_IICWriteReg16(0xD0, 0x01DF);   //LVDS_DATA_STREAM_LATENCY
    MTV_IICWriteReg16(0xD1, 0x0164);   //LVDS_INTERNAL_SYNC
    MTV_IICWriteReg16(0xD2, 0x0001);   //LVDS_USE_10BIT_PIXELS
    MTV_IICWriteReg16(0xD3, 0x0000);   //STEREO_ERROR_CONTROL
    MTV_IICWriteReg16(0xD4, 0x0000);   //INTERLACE_FIELD_VBLANK
    MTV_IICWriteReg16(0xD5, 0x0104);   //IMAGE_CAPTURE_NUM
    MTV_IICWriteReg16(0xD6, 0x0000);   //ANALOG_CONTROLS
    MTV_IICWriteReg16(0xD7, 0x0000);   //AB_PULSE_WIDTH_REG
    MTV_IICWriteReg16(0xD8, 0x0000);   //TX_PULLUP_PULSE_WIDTH_REG
    MTV_IICWriteReg16(0xD9, 0x0000);   //RST_PULLUP_PULSE_WIDTH_REG
    MTV_IICWriteReg16(0xF0, 0x0000);   //NTSC_FV_CONTROL
    MTV_IICWriteReg16(0xFE, 0xBEEF);   //NTSC_HBLANK
}

/*************************************************************************
 *************************************************************************
 *                            摄像头SCCB底层驱动
 *************************************************************************
*************************************************************************/

/*************************************************************************
* 函数名称: void MTV_IICWriteReg16(uint8_t reg, uint16_t val)
* 功能说明: 写寄存器
* 参数说明: reg 寄存器; val 数值
* 函数返回: 无
* 修改时间: 2020年10月08日
* 备    注:
*************************************************************************/
void MTV_IICWriteReg16(uint8_t reg, uint16_t val)
{
	SCCB_RegWrite(MT9V034_I2C_ADDR, reg, val);
}

/*************************************************************************
* 函数名称: void SCCB_Init(void)
* 功能说明: 配置SCCB所用引脚为GPIO功能，暂时不配置数据方向
* 参数说明: 无
* 函数返回: 无
* 修改时间: 2020年10月08日
* 备    注:
*************************************************************************/
void SCCB_Init(void)
{
	//GPIO_PinInit(SCCB_SCL_PIN, GPI_UP, 1);//配置为GPIO功能
	//GPIO_PinInit(SCCB_SDA_PIN, GPI_UP, 1);//配置为GPIO功能
	//PORT_SetPinMux(BOARD_INITPINS_CAMSCL_GPIO, BOARD_INITPINS_CAMSCL_PIN, kPORT_MuxAsGpio);
	//PORT_SetPinMux(BOARD_INITPINS_CAMSDA_GPIO, BOARD_INITPINS_CAMSDA_PIN, kPORT_MuxAsGpio);
	PORT_SetPinMux(BOARD_INITPINS_CAMSCL_PORT, BOARD_INITPINS_CAMSCL_PIN, kPORT_MuxAsGpio);
	PORT_SetPinMux(BOARD_INITPINS_CAMSDA_PORT, BOARD_INITPINS_CAMSDA_PIN, kPORT_MuxAsGpio);
}

/*************************************************************************
* 函数名称: void SCCB_Wait(void)
* 功能说明: SCCB等待演示
* 参数说明: 无
* 函数返回: 无
* 修改时间: 2020年10月08日
* 备    注:
*************************************************************************/
void SCCB_Wait(void)
{
	uint16_t i=0;
	for(i=0;i<100;i++)
	{
		asm ("nop");
	}
}

/*************************************************************************
* 函数名称: void SCCB_Star(void)
* 功能说明: 启动函数
* 参数说明: 无
* 函数返回: 无
* 修改时间: 2020年10月08日
* 备    注:
*************************************************************************/
void SCCB_Star(void)
{
	//SCL_Out;
	SDA_Out;
	SCCB_Wait();
	SDA_High;
	SCL_High;
	SCCB_Wait();
	SDA_Low;
	SCCB_Wait();
	SCL_Low;
	SCCB_Wait();
}

/*************************************************************************
* 函数名称: void SCCB_Stop(void)
* 功能说明: 停止函数
* 参数说明: 无
* 函数返回: 无
* 修改时间: 2020年10月08日
* 备    注:
*************************************************************************/
void SCCB_Stop(void)
{
	//SCL_Out;
	SDA_Out;
	SCCB_Wait();
	SDA_Low;
	SCCB_Wait();
	SCL_High;
	SCCB_Wait();
	SDA_High;
	SCCB_Wait();
}

/*************************************************************************
* 函数名称: uint8 SCCB_SendByte(uint8 Data)
* 功能说明: SCCB发送字节函数
* 参数说明: 要发送的字节
* 函数返回: 应答信号
* 修改时间: 2020年10月08日
* 备    注:
*************************************************************************/
uint8_t SCCB_SendByte(uint8_t Data)
{
	uint8_t i;
	uint8_t Ack;

	SDA_Out;
	for(i=0; i<8; i++)
	{
		if(Data & 0x80)
		{
			SDA_High;
		}
		else
		{
			SDA_Low;
		}
		Data <<= 1;
		SCCB_Wait();
		SCL_High;
		SCCB_Wait();
		SCL_Low;
		SCCB_Wait();
	}
	SDA_High;
	SDA_In;
	SCCB_Wait();

	SCL_High;
	SCCB_Wait();
	Ack = SDA_Data;
	SCL_Low;
	SCCB_Wait();

	return Ack;
}
/***************************************************************

*
* 函数名称: uint8 SCCB_ReadByte(void)
* 功能说明: SCCB读取字节函数
* 参数说明:
* 函数返回: 读取字节
* 修改时间: 2020年10月08日
* 备 注:
***************************************************************/
uint8_t SCCB_ReadByte(void)
{
	uint8_t i;
	uint8_t byte;

	//SCL_Out;
	SDA_In;	//使能输入
	for( i=0; i<8; i++)
	{
		SCL_Low;
		SCCB_Wait();
		SCL_High;
		SCCB_Wait();
		byte = (byte<<1)|(SDA_Data & 1);
	}
	SCL_Low;
	SDA_Out;
	SCCB_Wait();

	return byte;
}

/***************************************************************
* 函数名称: static void SCCB_Ack(void)
* 功能说明: IIC有回复信号
* 参数说明:
* 函数返回: void
* 修改时间: 2020年10月08日
* 备 注:
***************************************************************/
static void SCCB_Ack(void)
{
	//SCL_Out;
	SDA_Out;
	SCL_Low;
	SDA_Low;
	SCCB_Wait();
	SCL_High;
	SCCB_Wait();
	SCL_Low;
	SCCB_Wait();
}

/***************************************************************
* 函数名称: static void SCCB_NAck(void)
* 功能说明: IIC无回复信号
* 参数说明:
* 函数返回: void
* 修改时间: 2020年10月08日
* 备 注:
***************************************************************/
static void SCCB_NAck(void)
{
	//SCL_Out;
	SDA_Out;
	SCL_Low;
	SCCB_Wait();
	SDA_High;
	SCCB_Wait();
	SCL_High;
	SCCB_Wait();
	SCL_Low;
	SCCB_Wait();
}

/*************************************************************************
* 函数名称: void SCCB_RegWrite(uint8_t Device, uint8_t Address, uint16_t Data)
* 功能说明: 向设备写数据
* 参数说明: 要发送的字节
* 函数返回: 应答信号
* 修改时间: 2020年10月08日
* 备    注:
*************************************************************************/
void SCCB_RegWrite(uint8_t Device, uint8_t Address, uint16_t Data)
{
	uint8_t i;
	uint8_t Ack;

	for( i=0; i<3; i++)
	{
		SCCB_Star();
		Ack = SCCB_SendByte(Device);
		if( Ack == 1 )
		{
		  continue;
		}

		Ack = SCCB_SendByte(Address);
		if( Ack == 1 )
		{
		  continue;
		}

		Ack = SCCB_SendByte((uint8_t)(Data>>8));
		Ack = SCCB_SendByte((uint8_t)Data);
		if( Ack == 1 )
		{
		  continue;
		}

		SCCB_Stop();
		if( Ack == 0 ) break;
	}
}

/***************************************************************
* 函数名称: uint8_t SCB_RegRead(uint8_t Device, uint8_t Address, uint16_t *Data)
* 功能说明: 读取数据
* 参数说明:
* 函数返回: void
* 修改时间: 2020年10月08日
* 备 注:
***************************************************************/
uint8_t SCCB_RegRead(uint8_t Device, uint8_t Address, uint16_t *Data)
{
	uint8_t Ack = 0;
	Device = Device<<1;

	SCCB_Star();
	Ack += SCCB_SendByte(Device);
	Ack += SCCB_SendByte(Address);

	SCCB_Star();
	Ack += SCCB_SendByte(Device + 1);

	*Data = SCCB_ReadByte();
	SCCB_Ack();
	*Data = *Data<<8;

	*Data += SCCB_ReadByte();
	SCCB_NAck();

	SCCB_Stop();

	return  Ack;
}

/***************************************************************
* 函数名称: uint8_t SCCB_Probe(uint8_t chipAddr)
* 功能说明: 查询该地址的设备是否存在
* 参数说明: 芯片IIC地址
* 函数返回: void
* 修改时间: 2020年10月08日
* 备 注:
***************************************************************/
uint8_t SCCB_Probe(uint8_t chipAddr)
{
	uint8_t err;
	err = 0;
	chipAddr <<= 1;

	SCCB_Star();
	err = SCCB_SendByte(chipAddr);
	SCCB_Stop();

	return err;
}
