#ifndef __ADXL345_H
#define __ADXL345_H

#include "stdint.h"

#define ADLX345ADDR						        0xA6		////ALT  ADDRESS引脚接地时地址为0xA6，接电源时地址为0x3A

#define ADLX345ID							    0xE5		//adlx345器件ID


/* 活动和敲击检测位 */
#define ADLX345_TAP_Z							(1 << 0)
#define ADLX345_TAP_Y							(1 << 1)
#define ADLX345_TAP_X							(1 << 2)
#define ADLX345_SLEEP							(1 << 3)
#define ADLX345_ACT_Z						    (1 << 4)
#define ADLX345_ACT_Y							(1 << 5)
#define ADLX345_ACT_X							(1 << 6)


/* 中断类型检测位 */
#define ADLX345_OverrunInt						(1 << 0)
#define ADLX345_WatermarkInt					(1 << 1)
#define ADLX345_FREE_FALLInt					(1 << 2)
#define ADLX345_InactivityInt					(1 << 3)
#define ADLX345_ActivityInt						(1 << 4)
#define ADLX345_DOUBLE_TAPInt					(1 << 5)
#define ADLX345_SINGLE_TAPInt					(1 << 6)
#define ADLX345_DATA_READYInt					(1 << 7)


#define ADLX345_DEVID_REG						0x00		//器件ID地址
#define ADLX345_THRESH_TAP_REG				    0x29		//敲击阈值
#define ADLX345_OFSX_REG						0x1E		//X轴偏移
#define ADLX345_OFSY_REG						0x1F		//Y轴偏移
#define ADLX345_OFSZ_REG						0x20		//Z轴偏移
#define ADLX345_DUR_REG							0x21		//敲击持续时间
#define ADLX345_Latent_REG						0x22		//敲击延迟
#define ADLX345_Window_REG						0x23		//敲击窗口
#define ADLX345_THRESH_ACT_REG				    0x24		//活动阈值
#define ADLX345_THRESH_INACT_REG			    0x25		//静止阈值
#define ADLX345_TIME_INACT_REG				    0x26		//静止时间
#define ADLX345_ACT_INACT_CTL_REG			    0x27		//轴使能控制活动和静止检测
#define ADLX345_THRESH_FF_REG					0x28		//自由落体阈值
#define ADLX345_TIME_FF_REG						0x29		//自由落体时间
#define ADLX345_TAP_AXES_REG					0x2A		//单击/双击轴控制
#define ADLX345_ACT_TAP_STATUS_REG		        0x2B		//单击/双击源
#define ADLX345_BW_RATE_REG						0x2C		//数据速率及功率模式控制
#define ADLX345_POWER_CTL_REG					0x2D		//省电特性控制
#define ADLX345_INT_ENABLE_REG				    0x2E		//中断使能控制
#define ADLX345_INT_MAP_REG						0x2F		//中断映射控制
#define ADLX345_INT_SOURCE_REG				    0x30		//中断源
#define ADLX345_DATA_FORMAT_REG				    0x31		//数据格式控制
#define ADLX345_DATAX0_REG						0x32		//X轴数据0
#define ADLX345_DATAX1_REG						0x33		//X轴数据1
#define ADLX345_DATAY0_REG						0x34		//Y轴数据0
#define	ADLX345_DATAY1_REG						0x35		//Y轴数据1
#define ADLX345_DATAZ0_REG						0x36		//Z轴数据0
#define ADLX345_DATAZ1_REG						0x37		//Z轴数据1
#define ADLX345_FIFO_CTL_REG					0x38		//FIFO控制
#define ADLX345_FIFO_STATUS_REG				    0x39		//FIFO状态


#define ADXL345_OK						        0
#define ADXL345_ERR_ID				            1

/* 
 * 敲击检测重力加速度计算
 * THRESH_TAP_REG寄存器使用
 * 取值范围为0.0625g - 16g
 */
#define KnockGravity(x)				(x*1000/62.5 - 1)

/*
 * X、Y、Z轴偏移量补偿计算
 * OFSX_REG、OFSY_REG、OFSZ_REG寄存器使用
 * 取值范围为0.0156g - 2g
 */
#define OffsetValue(x)				(x*1000/15.6 - 1)

/*
 * 敲击持续时间阈值
 * DUR_REG寄存器使用
 * 时间为x(us),且x为625倍数
 * 当返回值为0时禁止单/双击功能
 */
#define DURValueUs(x)						(x/625)


/*
 * 从敲击事件检测到时间窗口（由Window寄存器定义）开始
	 的等待时间，在此期间，能检测出可能的第二次敲击事件
 * Latent_REG寄存器使用
 * 时间为x(ms)且x为1.25倍数
 * 当返回值为0时禁止双击功能
 */
#define LatentValueMs(x)				(x/1.25)

/*
 * 延迟时间(由Latent寄存器确定)期满后的时间量，在此
	 期间，能开始进行第二次有效敲击
 * Window_REG寄存器使用
 * 时间为x(ms)且x为1.25倍数
 * 当返回值为0时禁止双击功能 
 */
#define WindowValueMs(x)				(x/1.25)

/*
 * 保存检测活动的阈值
 * THRESH_ACT_REG寄存器使用
 * 取值范围为0.0625g - 16g 
 */
#define THRESH_ACTValue(x)			(x*1000/62.5 - 1)


/* 
 * 保存检测静止的阈值
 * THRESH_INACT_REG寄存器使用
 * 取值范围为0.0625g - 16g
 */
#define THRESH_INACTValue(x)		(x*1000/62.5 - 1)


/*
 * 加速度的时间量必须小于THRESH_INACT寄存器的
	 值，以宣布静止
 * TIME_INACT_REG寄存器使用
 * 时间为x(s)且x为1倍数
 */
#define TIME_INACTValue(x)			(x)


/*
 * 交流直流工作模式
 * ACT_INACT_CTL_REG使用
 */
typedef enum __AcDcMode{
	 dc = 0x00,
	 ac = 0x88,
 }AcDcMode;

/*
 * 活动中断使能方式
 * ACT_INACT_CTL_REG使用
 */
typedef enum __ActXYZ{
	act_null = 0x00,
	act_x = 0x11,
	act_y = 0x22,
	act_z = 0x44,
}ActXYZ;



/*
 * ADXL345通信速率
 * 正常功耗以下所有速率都能使用
 * 低功耗模式只能支持12.5Hz - 400Hz
 */
typedef enum __ModeRate{
	HZ0P10 	= 0x00,			//0.10Hz
	HZ0P20 	= 0x01,			//0.20Hz
	HZ0P39 	= 0x02,
	HZ0P78 	= 0x03,
	HZ1P56 	= 0x04,
	HZ3P13 	= 0x05,
	HZ6P25 	= 0x06,
	HZ12P5 	= 0x07,
	HZ25 	= 0x08,
	HZ50 	= 0x09,
	HZ100 	= 0x0A,
	HZ200 	= 0x0B,
	HZ400 	= 0x0C,
	HZ800 	= 0x0D,
	HZ1600 	= 0x0E,
	HZ3200 	= 0x0F,
}ModeRate;


/* 是否使用低功耗模式 */
typedef enum __LowPower{
	LPDisable = 0x00,
	LPEnable = 0x01
}LowPower;

/*
 * 芯片使能
 * StandbyMode : 待机模式
 * MeasuringMode : 检测模式
 */
typedef enum __PowerCTL{
	StandbyMode = 0x00,
	MeasuringMode = 0x01,
}PowerCTL;

/*
 * 中断模式使能
 */
typedef enum __IntEnable{
	IntDisable = 0x00,
	Overrun = 0x01,
	Watermark = 0x02,
	FREE_FALL = 0x04,
	Inactivity = 0x08,
	Activity = 0x10,
	DOUBLE_TAP = 0x20,
	SINGLE_TAP = 0x40,
	DATA_READY = 0x80,
}IntEnable;

/* 
 * 中断输出管脚配置 
 * 默认输出INT1管脚
 * 这里如果进行配置则配置到INT2管脚
 */
typedef enum __IntMap{
	AllMapINT1 = 0x00,
	OverrunMap = 0x01,
	WatermarkMap = 0x02,
	FREE_FALLMap = 0x04,
	InactivityMap = 0x08,
	ActivityMap = 0x10,
	DOUBLE_TAPMap = 0x20,
	SINGLE_TAPMap = 0x40,
	DATA_READYMap = 0x80,
}IntMap;

/*
 * 是否使能自检测
 */
typedef enum __SelfTest{
	STDisable = 0x00,
	STEnable = 0x01,
}SelfTest;

/*
 * SPI使用模式
 */
typedef enum __SpiMode{
	Spi3Bus = 0x00,
	Spi4Bus = 0x01,
}SpiMode;

/*
 * 中断输出电平
 */
typedef enum __IntOutPut{
	PinHight = 0x00,
	PinLow = 0x01,
}IntOutPut;

/*
 * 采集精度
 */
typedef enum __ColAccuracy{
	Bit10 = 0x00,
	Bit13 = 0x01,
}ColAccuracy;

/*
 * 加速度检测范围
 */
typedef enum DetectionRange{
	G2 = 0x00,
	G4 = 0x01,
	G8 = 0x02,
	G16 = 0x03,
}DetectionRange;




struct Adxl345Device{
	AcDcMode 				AcdcMode;		//电源工作方式
	ActXYZ					Act;			//活动检测方式
	ModeRate				Rate;			//通信速率
	LowPower  			    LowMode;			//低功耗使能
	PowerCTL				PCtl;			//芯片工作方式
	IntEnable 			    IntNvic;			//中断类型使能
	IntMap					IntMapIO;		//中断管脚配置
	SelfTest  			    SelfTestMode;	    //自测力使能
	SpiMode					Spi;			//SPI工作方式
	IntOutPut 			    IntIo;				//中断输出电平
	ColAccuracy 		    Accuracy;			//采集精度
	DetectionRange	        Range;				    //加速度检测范围
};


struct Adxl345Data{
	float x;
	float y;
	float z;
};

struct adxl345_i2c_function{
	void (*i2c_read_data)(uint8_t devaddr,uint8_t addr,uint8_t *data,uint8_t len);		//I2C读取数据
	void (*i2c_write_data)(uint8_t devaddr,uint8_t addr,uint8_t *data,uint8_t len);		//I2C写数据
};


uint8_t Adxl345Init(struct adxl345_i2c_function *fun);
void Adxl345ReadData(struct Adxl345Data *p);
#endif