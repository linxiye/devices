#ifndef __MAX17043_H
#define __MAX17043_H

#include "stdint.h"

#define MAX17043ADDR					    0x6C			//MAX17043的I2C地址

#define MAX17043_VCELL_REG					0x02			/*MAX17043的ADC采集存储寄存器,
															 * 0x02地址为高8位，0x03地址为低8位且低4为位无效
															 */											
#define MAX17043_SOC_REG					0x04			//MAX17043换算到的电池百分比电量(0x04为1%精度 0x05为1/256%精度)
#define MAX17043_MODE_REG					0x06			//模式寄存器
#define MAX17043_VERSION_REG				0x08			//芯片版本存储寄存器
#define MAX17043_CONFIG_REG					0x0C			//芯片配置寄存器
#define MAX17043_COMMAND_REG				0xFE			//芯片特殊命令寄存器


#define MAX17043_CONFIG_DATA				0x97			//配置值
#define MAX17043_RESET_CMD					0x5400			//芯片复位命令
#define MAX17043_QUICK_START				0x4000			//快速启动

#define MAX17043_OK                         0
#define MAX17043_ERROR                      1

struct max17043_i2c_function{
	void (*delayms)(uint32_t ms);
	void (*i2c_read_data)(uint8_t devaddr,uint8_t addr,uint8_t *data,uint8_t len);		//I2C读取数据
	void (*i2c_write_data)(uint8_t devaddr,uint8_t addr,uint8_t *data,uint8_t len);		//I2C写数据
};

uint8_t Max17043GetBattery(void);
uint16_t Max17043GetVol(void);
uint8_t Max17043Init(struct max17043_i2c_function *fun);

#endif

