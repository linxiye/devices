#include "adxl345.h"
#include "stdio.h"


static struct adxl345_i2c_function *adxl345_fun;


uint8_t Adxl345Init(struct adxl345_i2c_function *fun)
{
	uint8_t data;
	adxl345_fun = fun;
	adxl345_fun->i2c_read_data(ADLX345ADDR,ADLX345_DEVID_REG,&data,1);
	if(data != ADLX345ID) return ADXL345_ERR_ID;
	data = 0x0B;
	adxl345_fun->i2c_write_data(ADLX345ADDR,ADLX345_DATA_FORMAT_REG,&data,1);
	data = 0x08;
	adxl345_fun->i2c_write_data(ADLX345ADDR,ADLX345_BW_RATE_REG,&data,1);
	data = 0x08;
	adxl345_fun->i2c_write_data(ADLX345ADDR,ADLX345_POWER_CTL_REG,&data,1);	
	data = 0x00;
	adxl345_fun->i2c_write_data(ADLX345ADDR,ADLX345_INT_ENABLE_REG,&data,1);	
	adxl345_fun->i2c_write_data(ADLX345ADDR,ADLX345_OFSX_REG,&data,1);	
	adxl345_fun->i2c_write_data(ADLX345ADDR,ADLX345_OFSY_REG,&data,1);	
	adxl345_fun->i2c_write_data(ADLX345ADDR,ADLX345_OFSZ_REG,&data,1);	
	return ADXL345_OK;
}


void Adxl345ReadData(struct Adxl345Data *p)
{
	uint8_t data[6];
	int16_t x,y,z; 
	adxl345_fun->i2c_read_data(ADLX345ADDR,ADLX345_DATAX0_REG,data,6);
	x = (data[1] << 8) | data[0];p->x = (float)x*3.9/1000 * 9.8;
	y = (data[3] << 8) | data[2];p->y = (float)y*3.9/1000 * 9.8;
	z = (data[5] << 8) | data[4];p->z = (float)z*3.9/1000 * 9.8;
}
