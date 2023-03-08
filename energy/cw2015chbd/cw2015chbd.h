#ifndef __CW2015CHBD_H
#define __CW2015CHBD_H

#include "stdint.h"

#define CW2015CHBD_DEVICE_ADDR              0xC4        /* I2C DEVICE ADDR*/

#define REG_VERSION             0x0
#define REG_VCELL               0x2
#define REG_SOC                 0x4
#define REG_RRT_ALERT           0x6
#define REG_CONFIG              0x8
#define REG_MODE                0xA
#define REG_BATINFO				0x10

#define CW2015CHBD_OK                         0
#define CW2015CHBD_ERROR                      1

#define MODE_SLEEP_MASK         (0x3<<6)
#define MODE_SLEEP              (0x3<<6)
#define MODE_NORMAL             (0x0<<6)
#define MODE_QUICK_START        (0x3<<4)
#define MODE_RESTART            (0xf<<0)
#define CONFIG_UPDATE_FLG       (0x1<<1)

#define ATHD                    (0x0<<3)        //ATHD = 0%
#define UI_FULL             100

#define SIZE_BATINFO        64

#define BATTERY_DOWN_MIN_CHANGE_SLEEP 1800      // the min time allow battery change quantity when run 30min

struct cw2015chbd_i2c_function{
	void (*delayus)(uint32_t us);
	void (*delayms)(uint32_t ms);
	void (*i2c_read_data)(uint8_t devaddr,uint8_t addr,uint8_t *data,uint8_t len);		//I2C读取数据
	void (*i2c_write_data)(uint8_t devaddr,uint8_t addr,uint8_t *data,uint8_t len);		//I2C写数据
};

//****************************struct*********************************/
typedef struct tagSTRUCT_CW_BATTERY {
	unsigned char usb_online;
	unsigned int capacity;
	unsigned int voltage;
	unsigned char alt;
}STRUCT_CW_BATTERY;

uint8_t cw_por(void);
uint8_t Cw2015chbdInit(struct cw2015chbd_i2c_function *fun);
uint32_t Cw2015chbdGetVol(void);
uint32_t Cw2015chbdGetBattery(void);
uint8_t Cw2015chbdGetMode(void);
void Cw2015chbdSetMode(uint8_t data);
void Cw2015chbdSetSleepMode(void);
void Cw2015chbdSetWakeMode(void);
void cw_bat_tick_1s(void);
void Cw2015chbdSetUsb(uint8_t value);
#endif
