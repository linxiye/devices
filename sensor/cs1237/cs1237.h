#ifndef __CS1237_H
#define __CS1237_H

#include "stdint.h"

#define WRITE_CONFIG_CMD		0x65			//写配置寄存器命令
#define READ_CONFIG_CMD			0x56			//度配置寄存器命令


/*
 * 通道设置，共2位
 * 默认adc采集通道
 * 00：adc采集通道
 * 01：芯片保留
 * 10：温度采集通道
 * 11：芯片内短
 */
typedef enum CH_SEL{
	adc_ch = 0x00,
	retain_bit,
	temp_ch,
	short_circuit,
}ch_sel_t;

/*
 * 低噪声 PGA 放大器
 * 默认128倍放大
 * 00：1倍放大
 * 01：2倍放大
 * 10：64倍放大
 * 11：128倍放大
 */
typedef enum PGA_SEL{
	pga1 = 0x00,
	pga2,
	pga64,
	pga128,
}pga_sel_t;

/*
 * ADC 数据输出速率
 * 00：10Hz
 * 01：40Hz
 * 10：640Hz
 * 11：1280Hz
 */
typedef enum SPEED_SEL{
	adc10Hz = 0x00,
	adc40Hz,
	adc640Hz,
	adc1280Hz,
}speed_sel_t;

/*
 * 内部基准控制寄存器
 * 0：使用内部基准电压
 * 1：关闭内部基准电压（使用外部基准电压）
 */
typedef enum REFO{
	refo_on = 0x00,
	refo_off,
}refo_t;


/*
 * cs1237工作初始化结构体
 */
struct cs1237_fun{
	void (*delay_us)(uint32_t us);				//微妙延时函数
	void (*delay_ms)(uint32_t ms);				//毫秒延时函数
	void (*scl_output_data)(uint8_t value);		//时钟线IO输出函数
	void (*set_sda_input)(void);				//设置数据线为输入函数
	void (*set_sda_output)(void);				//设置数据线为输出函数
	void (*sda_output_data)(uint8_t value);		//数据线IO输出函数
	uint8_t (*get_sda_data)(void);				//获取数据线电平

	/* 中断开关函数(设置为下降沿中断) */
	void (*sda_irq_enable)(void);				//开启sda中断
	void (*sda_irq_disable)(void);				//关闭sda中断
};

/*
 * cs1237配置结构体
 */
struct cs1237_config{
	ch_sel_t			ch;			//选择通道
	pga_sel_t			pga;		//选择低噪声PGA放大器倍数
	speed_sel_t			speed;	//选择ADC数据输出速率
	refo_t				refo;		//是否关闭内部基准电压
};


/*
 * CS1237进入Power down 模式
 * 这时会关掉芯片所有电路，功耗接近0
 */
void cs1237_powerdown(void);

/*
 * 当芯片进入低功耗模式后
 * SCLK 重新回到低电平时，芯片会重新进入正常工作状态
 */
void cs1237_normal(void);

/*
 * 开始启动数据采集
 */
void cs1237_start(void);

/*
 * 关闭数据采集
 */
void cs1237_stop(void);
/*
 * 读取CS1237的ADC采集值
 * 此读取必须为中断读取
 * 返回值：CS1237ADC值（没有进行转换）
 */
uint32_t cs1237_read_value(void);


/*
 * CS1237初始化函数
 * 该函数需要用户给出const struct cs1237_dev操作函数结构体
	 里面包含了CS1237基本操作的函数定义
 * 以及需要用户配置struct cs1237_config配置函数结构体
	 会根据用户的配置在初始化过程中写入CS1237的配置寄存器
 * @dev:操作函数配置
 * @config:CS1237功能配置
 * 返回值：	-1：初始化失败
			0：初始化成功
 */
int cs1237_init(const struct cs1237_fun *dev,struct cs1237_config config);


/*
 * cs1237 sda 中断回调函数
 * 触发cs1237 sda中断后
 * 调用此函数读取数据
 * 这部分需要用户在中断出发后
 * 运行函数里调用来触发采集(不建议放到中断函数里采集)
 */
void cs1237_sda_irq_callback(void);

/*
 * 获取CS1237配置参数
 */
uint8_t cs1237_read_config(void);
#endif
