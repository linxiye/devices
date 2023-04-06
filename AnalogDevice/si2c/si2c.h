#ifndef __SI2C_H
#define __SI2C_H

#include "stdint.h"

/*
 * SDA�ܽŲ��������ṹ��
 */
struct sda_pin {
	void (*sda_input) (void);
	void (*sda_output)(void);
	void (*sda_set_value) (uint8_t value);
	uint8_t (*sda_read_value)(void);
};

/*
 * SCL�ܽŲ��������ṹ��
 */
struct scl_pin {
	void (*scl_output)(void);
	void (*scl_set_value) (uint8_t value);
};

/*
 * I2C�豸�ṹ��
 */
struct si2c_dev {
	struct sda_pin sda;
	struct scl_pin scl;
	void (*usdelay)(uint32_t usdelay);
};


/*
 * ���i2c��ʼ��
 * ��ȡSDA��SCLIO�ڲ���
 * ��ʼ��SDA��SCLΪ�ߵ�ƽ��IIC����SDA��SCL��Ϊ�ߵ�ƽ��
 * @fops:���IO���ƺ����Լ�΢����ʱ�������ʼ������
 */
void si2c_init(struct si2c_dev *fops);



/*
 * I2C��������
 * �����豸�ɱ�д�Լ���I2C������ʽ
 */
void si2c_start(void);
void si2c_stop(void);
int si2c_wait_ack(void);
void si2c_ack(void);
void si2c_nack(void);
void si2c_send_bit(uint8_t data);
uint8_t si2c_read_bit(uint8_t ack);

/*
 * �򵥵Ķ�ȡ�Ĵ�������
 * ֻ�ܶ�ȡ8bit�������͵�����
 * ��bmp280�豸��24c02�豸����
 * @devaddr:��ȡ�豸��IIC��ַ
 * @memaddr:��ȡ�ļĴ�����ַ
 */
uint8_t si2c_read_data(uint8_t devaddr,uint8_t memaddr);

/*
 * �򵥵���Ĵ���д����
 * ֻ��д��8bit�������͵�����
 * ��bmp280�豸��24c02�豸����
 * @devaddr:д���豸��IIC��ַ
 * @memaddr:д��ļĴ�����ַ
 * @data:д�������
 */
void si2c_write_data(uint8_t devaddr,uint8_t memaddr,uint8_t data);

#endif

