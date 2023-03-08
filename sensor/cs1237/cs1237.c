#include "cs1237.h"
#include "stdio.h"
#include "rtthread.h"

static struct cs1237_fun *cs1237;
static uint32_t cs1237_value;

#define ONE_CLK			cs1237->scl_output_data(1);cs1237->delay_us(30);cs1237->scl_output_data(0);cs1237->delay_us(30)

/*
 * CS1237进入Power down 模式
 * 这时会关掉芯片所有电路，功耗接近0
 */
void cs1237_powerdown(void)
{
	cs1237->scl_output_data(1);
	cs1237->delay_ms(200);
}

/*
 * 当芯片进入低功耗模式后
 * SCLK 重新回到低电平时，芯片会重新进入正常工作状态
 */
void cs1237_normal(void)
{
	cs1237->scl_output_data(0);
	cs1237->delay_ms(200);
}

/*
 * 开始启动数据采集
 */
void cs1237_start(void)
{
	cs1237->sda_irq_enable();
}

void cs1237_stop(void)
{
    cs1237->sda_irq_disable();
}

/*
 * CS1237数据发送
 * @data:发送的数据
 * 发送从高位到低位按顺序发送7字节
 */
static void cs1237_send_cmd(uint8_t cmd)
{
	uint8_t value = 0;
	cs1237->set_sda_output();
	cs1237->sda_output_data(0);
	for(int i = 0;i<7;i++)
	{
		value = (cmd >> (6 - i)) & 0x01;
		cs1237->scl_output_data(1);cs1237->delay_us(30); 
		cs1237->sda_output_data(value);
		cs1237->scl_output_data(0);cs1237->delay_us(30);
	}
}

/* 
 * CS1237工作前准备
 * 先拉高SDA,设置SDA为输入
 * 在拉低SCL等待SDA被拉低
 * 如果超过5次还没被拉低则判断失败
 * 返回值：-1：失败
 * 				  0：成功
 */
static uint8_t cs1237_ready(void)
{
	uint8_t count = 0;
	/* SDA拉高后设置为输入，SCL拉低，等待SDA被拉低 */
	cs1237->scl_output_data(0);
	cs1237->set_sda_output();
	cs1237->sda_output_data(1);
	
	cs1237->set_sda_input();

	while(cs1237->get_sda_data() == 1)
	{
		cs1237->delay_ms(5);
		count++;
		if(count > 150)										/* 超时没拉低退出 */
		{
			cs1237->scl_output_data(1);
			cs1237->set_sda_output();
			cs1237->sda_output_data(1);
			return -1;
		}
	}
	return 0;
}

/*
 * CS1237配置寄存器设置
 * 会配置CS1237的通道、PGA、
   内部基准、ADC数据输出速率
 */
static uint8_t cs1237_write_config(uint8_t data)
{
	uint8_t value = 0;
	/* 等待cs1237数据准备 */
	if(cs1237_ready() != 0)
		return -1;
	/* 发送29个CLK */
	for(int i = 0; i < 29;i++)
	{
		ONE_CLK;
	}
	/* 发送0x65 */
	cs1237_send_cmd(WRITE_CONFIG_CMD);
	ONE_CLK;/* 37 */
	for(int i = 0; i < 8; i++)// 38 - 45个脉冲了，读取数据
  {
		value = (data >> (7 - i)) & 0x01;
    	cs1237->scl_output_data(1);cs1237->delay_us(30);
		cs1237->sda_output_data(value);
		cs1237->scl_output_data(0);cs1237->delay_us(30);
  }
	ONE_CLK;/* 46 */
	
	return 0;
}

/*
 * CS1237配置寄存器读取
 * 返回值：现在CS1237的配置值
 */
uint8_t cs1237_read_config(void)
{
	uint8_t data = 0;
	
	/* 等待cs1237数据准备 */
	if(cs1237_ready() != 0)
		return -1;
	
	/* 发送29个CLK */
	for(int i = 0; i < 29;i++)
	{
		ONE_CLK;
	}
	
	/* 发送0x56 */
	cs1237_send_cmd(READ_CONFIG_CMD);
	cs1237->sda_output_data(1);
	ONE_CLK;/* 37 */
	cs1237->set_sda_input();
	for(int i = 0; i < 8; i++)// 38 - 45个脉冲了，读取数据
  {
    ONE_CLK;
    data <<= 1;
    if(cs1237->get_sda_data() == 1)
      data++;
  }
	ONE_CLK;/* 46 */
	
	return data;
}

/*
 * 读取CS1237的ADC采集值
 * 此读取必须为中断读取
 * 返回值：0：CS1237ADC值（没有进行转换）
 */
static uint32_t cs1237_read_adc(void)
{
	uint32_t data = 0;
	for(int i = 0; i < 24; i++)//获取24位有效转换
	{
		cs1237->scl_output_data(1);cs1237->delay_us(1);
		data = data << 1;
		if(cs1237->get_sda_data() == 1)
			data++;
		cs1237->scl_output_data(0);cs1237->delay_us(1);
	}
	for(int i = 0; i < 3; i++)//一共输入27个脉冲
	{
		cs1237->scl_output_data(1);cs1237->delay_us(1);
		cs1237->scl_output_data(0);cs1237->delay_us(1);
	}
	/* 恢复待读数据状态 */
	cs1237->set_sda_output();
	cs1237->sda_output_data(1);	
	cs1237->set_sda_input();
	cs1237->scl_output_data(0);
	return data;
}


/*
 * CS1237初始化函数
 * 该函数需要用户给出const struct cs1237_dev操作函数结构体
	 里面包含了CS1237基本操作的函数定义
 * 以及需要用户配置struct cs1237_config配置函数结构体
	 会根据用户的配置在初始化过程中写入CS1237的配置寄存器
 * @dev:操作函数配置
 * @config:CS1237功能配置
 * 返回值：-1：初始化失败
			0：初始化成功
 */
int cs1237_init(const struct cs1237_fun *dev,struct cs1237_config config)
{
	uint8_t	data = (config.refo << 6) | (config.speed << 4) | (config.pga << 2) | config.ch;
	cs1237 = (struct cs1237_fun *)dev;
	cs1237_powerdown();
	cs1237_normal();
	cs1237_write_config(data);//配置CS1237
//	rt_kprintf("cs1237:%#x %#X\r\n",data,cs1237_read_config());
	if(cs1237_read_config() != data)//如果读取的ADC配置出错，则返回失败
		return -1;
	return 0;
}


void cs1237_sda_irq_callback(void)
{
	cs1237->sda_irq_disable();
	cs1237_value = cs1237_read_adc();
	cs1237->sda_irq_enable();
}

uint32_t cs1237_read_value(void)
{
	return cs1237_value;
}
