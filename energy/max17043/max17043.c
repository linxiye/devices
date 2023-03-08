#include "max17043.h"
#include "stdio.h"

static struct max17043_i2c_function *max17043_fun;

uint16_t Max17043GetVol(void)
{
    uint8_t vol[2] = {0};
    uint16_t value;
    max17043_fun->i2c_read_data(MAX17043ADDR,MAX17043_VCELL_REG,vol,2);
    value = (vol[0] << 4) | ((vol[1] >> 4) & 0x0F);
    return value;
}

uint8_t Max17043GetBattery(void)
{
    uint8_t battery[2] = {0};
    max17043_fun->i2c_read_data(MAX17043ADDR,MAX17043_SOC_REG,battery,2);
    return battery[0];
}

void max17043_reset(void)
{
    uint8_t data[2] = {0x54,0x00};
    max17043_fun->i2c_write_data(MAX17043ADDR,MAX17043_COMMAND_REG,data,2);
}

void max17043_config(void)
{
    uint8_t data[2] = {0x97,0x1C};
    max17043_fun->i2c_write_data(MAX17043ADDR,MAX17043_CONFIG_REG,data,2);
}


uint8_t Max17043Init(struct max17043_i2c_function *fun)
{
    uint8_t data[2];
    max17043_fun = fun;
    max17043_fun->i2c_read_data(MAX17043ADDR,MAX17043_CONFIG_REG,data,2);
    if(data[0] != 0x97 || data[1] != 0x1C) return MAX17043_ERROR;
    max17043_fun->delayms(150);
    max17043_reset();
    /* 复位后启动需要0.5秒等待稳定 */
    max17043_fun->delayms(500);
    max17043_config();
    max17043_fun->delayms(550);
    return MAX17043_OK;
}

