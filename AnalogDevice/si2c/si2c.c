#include "si2c.h"
#include "stdio.h"

struct si2c_dev *si2c;

/*
 * 软件i2c初始化
 * 获取SDA和SCLIO口操作
 * 初始化SDA和SCL为高电平（IIC空闲SDA与SCL都为高电平）
 * @fops:相关IO控制函数以及微秒延时函数结初始化构体
 */
void si2c_init(struct si2c_dev *fops)
{
	si2c = fops;
	
	si2c->scl.scl_output();
	si2c->scl.scl_set_value(1);
	
	si2c->sda.sda_output();
	si2c->sda.sda_set_value(1);
}

/*
 * 产生I2C起始信号
 * 当SCL为高电平时，SDA从高电平转为低电平，表示I2C起始信号
 * 
	SCL:___________________________
							|				  |
							|				  |
							|  START  | 
	SDA:________|___			|
									\			|
									 \____|________

 * 产生起始信号之后因把SCL置0,方便后续传输
 */
void si2c_start(void)
{
	si2c->sda.sda_output();
	si2c->scl.scl_set_value(1);
	si2c->sda.sda_set_value(1);
	si2c->usdelay(4);
	si2c->sda.sda_set_value(0);
	si2c->usdelay(4);
	si2c->scl.scl_set_value(0);	/* 此步为准备I2C发送数据 */
}


/*
 * 产生I2C停止信号
 * 当SCL为高电平时，SDA从低电平转为高电平，表示I2C起始信号
 *
		SCL:___________________________
								|				 |
								|				 |
								|	 STOP	 |
		SDA:				|	    ___|_________
								|		 /
				________|___/
 */
void si2c_stop(void)
{
	si2c->sda.sda_output();
	si2c->scl.scl_set_value(0);
	si2c->sda.sda_set_value(0);
	si2c->usdelay(4);
	si2c->scl.scl_set_value(1);
	si2c->sda.sda_set_value(1);
}


/*
 * 等待回复ACK信号
 * 当SCL为高电平时,SDA为低电平时，表示ACK信号
 *
		SCL:         _________
							  /|			 |\
				_______/ |			 | \_______
								 |	ACK  |
		SDA:____		 |			 |    	
					  \		 |		 	 |	 
					   \___|_______|________
 */
int si2c_wait_ack(void)
{
	uint32_t timer = 0;
	si2c->sda.sda_set_value(1);
	si2c->usdelay(1);
	si2c->sda.sda_input();
	si2c->scl.scl_set_value(1);
	si2c->usdelay(1);
	while(si2c->sda.sda_read_value()){
		timer++;
		if(timer>250){
			si2c_stop();
			return -1;
		}
	}
	si2c->scl.scl_set_value(0);
	return 0;
}

/*
 * 产生ACK信号
 * 时序图与回复ACK信号一样
	 只是一个接受ACK一个发送ACK而已
 */
void si2c_ack(void)
{
	si2c->scl.scl_set_value(0);
	si2c->sda.sda_output();
	si2c->sda.sda_set_value(0);
	si2c->usdelay(2);
	si2c->scl.scl_set_value(1);
	si2c->usdelay(2);
	si2c->scl.scl_set_value(0);
}

/*
 * 产生NACK信号
 * 当SCL为高电平时,SDA为高电平时，表示NACK信号
 *
		SCL:         _________
							  /|			 |\
				_______/ |			 | \_______
								 |  NACK |
		SDA:_________|_______|_________  	
					  

 */
void si2c_nack(void)
{
	si2c->scl.scl_set_value(0);
	si2c->sda.sda_output();
	si2c->sda.sda_set_value(1);
	si2c->usdelay(2);
	si2c->scl.scl_set_value(1);
	si2c->usdelay(2);
	si2c->scl.scl_set_value(0);
}


/*
 * 发送1比特数据
 */
void si2c_send_bit(uint8_t data)
{
	uint8_t i;
	si2c->sda.sda_output();
	si2c->scl.scl_set_value(0);
	for(i = 0;i < 8;i++){				/* 从高位往低位发送 */
		si2c->sda.sda_set_value((data & 0x80) >> 7);
		data = data << 1;
		si2c->usdelay(2);
		si2c->scl.scl_set_value(1);
		si2c->usdelay(2);
		si2c->scl.scl_set_value(0);
		si2c->usdelay(2);
	}
}

/*
 * 接受1比特数据
 */
uint8_t si2c_read_bit(uint8_t ack)
{
	uint8_t value = 0;
	si2c->sda.sda_input();
	for(int i = 0;i<8;i++){					/* 先接受高位 */
		si2c->scl.scl_set_value(0);
		si2c->usdelay(2);
		si2c->scl.scl_set_value(1);
		value = value << 1;
		if(si2c->sda.sda_read_value())value++;
		si2c->usdelay(1);
	}
	
	if(ack)si2c_ack();	/* 发送ACK信号 */
	else si2c_nack();		/* 发送NAK信号 */
	
	return value;
}

/*
 * 读取寄存器数据
 * @devaddr:读取设备的IIC地址
 * @memaddr:读取的寄存器地址
 * |产生起始信号|发送设备IIC地址|接受ACK|发送寄存器地址|接受ACK|产生起始信号|发送IIC地址+1（表示读取数据）|接受ACK|读取数据|产生停止信号|
 */
uint8_t si2c_read_data(uint8_t devaddr,uint8_t memaddr)
{
	uint8_t data = 0;
	si2c_start();
	si2c_send_bit(devaddr);
	si2c_wait_ack();
	si2c_send_bit(memaddr);
	si2c_wait_ack();
	si2c_start();
	si2c_send_bit(devaddr + 1);
	si2c_wait_ack();
	data = si2c_read_bit(0);
	si2c_stop();
	return data;
}

/*
 * 向寄存器写数据
 * @devaddr:写入设备的IIC地址
 * @memaddr:写入的寄存器地址
 * @data:写入的数据
 * |产生起始信号|发送设备IIC地址|接受ACK|发送寄存器地址|接受ACK|发送写入寄存器的数据|接受ACK|产生停止信号|
 */
void si2c_write_data(uint8_t devaddr,uint8_t memaddr,uint8_t data)
{
	si2c_start();
	si2c_send_bit(devaddr);
	si2c_wait_ack();
	si2c_send_bit(memaddr);
	si2c_wait_ack();
	si2c_send_bit(data);
	si2c_wait_ack();
	si2c_stop();
}
