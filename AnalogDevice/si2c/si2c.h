#ifndef __SI2C_H
#define __SI2C_H

#include "stdint.h"

/*
 * SDA管脚操作函数结构体
 */
struct sda_pin {
	void (*sda_input) (void);
	void (*sda_output)(void);
	void (*sda_set_value) (uint8_t value);
	uint8_t (*sda_read_value)(void);
};

/*
 * SCL管脚操作函数结构体
 */
struct scl_pin {
	void (*scl_output)(void);
	void (*scl_set_value) (uint8_t value);
};

/*
 * I2C设备结构体
 */
struct si2c_dev {
	struct sda_pin sda;
	struct scl_pin scl;
	void (*usdelay)(uint32_t usdelay);
};


/*
 * 软件i2c初始化
 * 获取SDA和SCLIO口操作
 * 初始化SDA和SCL为高电平（IIC空闲SDA与SCL都为高电平）
 * @fops:相关IO控制函数以及微秒延时函数结初始化构体
 */
void si2c_init(struct si2c_dev *fops);



/*
 * I2C操作函数
 * 根据设备可编写自己的I2C交流方式
 */
void si2c_start(void);
void si2c_stop(void);
int si2c_wait_ack(void);
void si2c_ack(void);
void si2c_nack(void);
void si2c_send_bit(uint8_t data);
uint8_t si2c_read_bit(uint8_t ack);

/*
 * 简单的读取寄存器数据
 * 只能读取8bit数据类型的数据
 * 如bmp280设备、24c02设备数据
 * @devaddr:读取设备的IIC地址
 * @memaddr:读取的寄存器地址
 */
uint8_t si2c_read_data(uint8_t devaddr,uint8_t memaddr);

/*
 * 简单的向寄存器写数据
 * 只能写入8bit数据类型的数据
 * 如bmp280设备、24c02设备数据
 * @devaddr:写入设备的IIC地址
 * @memaddr:写入的寄存器地址
 * @data:写入的数据
 */
void si2c_write_data(uint8_t devaddr,uint8_t memaddr,uint8_t data);

#endif

