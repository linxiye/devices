#include "si2c.h"
#include "stdio.h"

struct si2c_dev *si2c;

/*
 * ���i2c��ʼ��
 * ��ȡSDA��SCLIO�ڲ���
 * ��ʼ��SDA��SCLΪ�ߵ�ƽ��IIC����SDA��SCL��Ϊ�ߵ�ƽ��
 * @fops:���IO���ƺ����Լ�΢����ʱ�������ʼ������
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
 * ����I2C��ʼ�ź�
 * ��SCLΪ�ߵ�ƽʱ��SDA�Ӹߵ�ƽתΪ�͵�ƽ����ʾI2C��ʼ�ź�
 * 
	SCL:___________________________
							|				  |
							|				  |
							|  START  | 
	SDA:________|___			|
									\			|
									 \____|________

 * ������ʼ�ź�֮�����SCL��0,�����������
 */
void si2c_start(void)
{
	si2c->sda.sda_output();
	si2c->scl.scl_set_value(1);
	si2c->sda.sda_set_value(1);
	si2c->usdelay(4);
	si2c->sda.sda_set_value(0);
	si2c->usdelay(4);
	si2c->scl.scl_set_value(0);	/* �˲�Ϊ׼��I2C�������� */
}


/*
 * ����I2Cֹͣ�ź�
 * ��SCLΪ�ߵ�ƽʱ��SDA�ӵ͵�ƽתΪ�ߵ�ƽ����ʾI2C��ʼ�ź�
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
 * �ȴ��ظ�ACK�ź�
 * ��SCLΪ�ߵ�ƽʱ,SDAΪ�͵�ƽʱ����ʾACK�ź�
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
 * ����ACK�ź�
 * ʱ��ͼ��ظ�ACK�ź�һ��
	 ֻ��һ������ACKһ������ACK����
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
 * ����NACK�ź�
 * ��SCLΪ�ߵ�ƽʱ,SDAΪ�ߵ�ƽʱ����ʾNACK�ź�
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
 * ����1��������
 */
void si2c_send_bit(uint8_t data)
{
	uint8_t i;
	si2c->sda.sda_output();
	si2c->scl.scl_set_value(0);
	for(i = 0;i < 8;i++){				/* �Ӹ�λ����λ���� */
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
 * ����1��������
 */
uint8_t si2c_read_bit(uint8_t ack)
{
	uint8_t value = 0;
	si2c->sda.sda_input();
	for(int i = 0;i<8;i++){					/* �Ƚ��ܸ�λ */
		si2c->scl.scl_set_value(0);
		si2c->usdelay(2);
		si2c->scl.scl_set_value(1);
		value = value << 1;
		if(si2c->sda.sda_read_value())value++;
		si2c->usdelay(1);
	}
	
	if(ack)si2c_ack();	/* ����ACK�ź� */
	else si2c_nack();		/* ����NAK�ź� */
	
	return value;
}

/*
 * ��ȡ�Ĵ�������
 * @devaddr:��ȡ�豸��IIC��ַ
 * @memaddr:��ȡ�ļĴ�����ַ
 * |������ʼ�ź�|�����豸IIC��ַ|����ACK|���ͼĴ�����ַ|����ACK|������ʼ�ź�|����IIC��ַ+1����ʾ��ȡ���ݣ�|����ACK|��ȡ����|����ֹͣ�ź�|
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
 * ��Ĵ���д����
 * @devaddr:д���豸��IIC��ַ
 * @memaddr:д��ļĴ�����ַ
 * @data:д�������
 * |������ʼ�ź�|�����豸IIC��ַ|����ACK|���ͼĴ�����ַ|����ACK|����д��Ĵ���������|����ACK|����ֹͣ�ź�|
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
