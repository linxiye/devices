#include "cw2015chbd.h"
#include "stdio.h"

/*电池建模信息，客户拿到自己电池匹配的建模信息后请替换*/
const unsigned char cw_bat_config_info[SIZE_BATINFO] = {
	0x15,0x7E,0x7F,0x69,0x4F,0x30,0x4F,0x61,
	0x58,0x46,0x4D,0x58,0x68,0x6B,0x5C,0x4D,
	0x43,0x39,0x32,0x2F,0x39,0x45,0x4E,0x58,
	0x54,0x5C,0x0A,0x3E,0x20,0x3F,0x63,0x78,
	0x81,0x82,0x84,0x83,0x3D,0x1A,0x6E,0x0A,
	0x03,0x52,0x24,0x5E,0x87,0x93,0x98,0x33,
	0x55,0x7F,0x98,0xA0,0x80,0x9E,0xB2,0xCB,
	0x2F,0x00,0x64,0xA5,0xB5,0x25,0xD0,0x09,
};

static uint8_t CHARGE = 0; //是否接充电器标志位，1为接充电器，0为拔出充电器
static unsigned int allow_charger_always_zero =0;
static unsigned char if_quickstart =0;
static unsigned char reset_loop =0;

static struct cw2015chbd_i2c_function *cw2015chbd_fun;

static STRUCT_CW_BATTERY   cw_bat;


void CW_Delay10ms(unsigned int ms) 
{
    for(int i=0; i<ms; i++){
        cw2015chbd_fun->delayms(10);
    }
}

void CW_Delay10us(unsigned char us)
{
    for(int i = 0;i<us;i++){
        cw2015chbd_fun->delayus(10);
    }
}

uint8_t cw_read(unsigned char PointReg,unsigned char *pData)
{
    cw2015chbd_fun->i2c_read_data(CW2015CHBD_DEVICE_ADDR,PointReg,pData,1);
    return 0;
}

uint8_t cw_write(unsigned char PointReg,unsigned char *pData)
{
    cw2015chbd_fun->i2c_write_data(CW2015CHBD_DEVICE_ADDR,PointReg,pData,1);
    return 0;
}


uint8_t cw_read_word(unsigned char point_reg,unsigned char *r_pdata, unsigned int length)
{
    cw2015chbd_fun->i2c_read_data(CW2015CHBD_DEVICE_ADDR,point_reg,r_pdata,length);
    return 0;
}

/*这个函数的作用是更新ic内的电池profile信息，一般只有在ic VDD掉电后再上电时才执行 
return 1 : i2c读写错， return 2 : 芯片处于sleep模式 return 3 : 写入的profile信息读出后与代码中的不一致*/
unsigned char cw_update_config_info(void)
{
	uint8_t ret = 0;
	unsigned char i;
	unsigned char reset_val;
	unsigned char reg_val;
	/* make sure no in sleep mode */
	ret = cw_read(REG_MODE, &reg_val);
	if(ret)
	{
		return 1;
	}
	if((reg_val & MODE_SLEEP_MASK) == MODE_SLEEP)
	{
		return 2;
	}
	/* update new battery info */
	for(i = 0; i < SIZE_BATINFO; i++)
	{
		reg_val = cw_bat_config_info[i];
		ret = cw_write(REG_BATINFO+i, &reg_val);
		if(ret)
		{
			return 1;
		}
	}

	/* readback & check */
	for(i = 0; i < SIZE_BATINFO; i++)
	{
		ret = cw_read(REG_BATINFO+i, &reg_val);
		if(ret)
		{
			return 1;
		}
		/* debug_log("[cw_update]: REG[%02X] = %02X", REG_BATINFO +i, reg_val); */
		if(reg_val != cw_bat_config_info[i])
		{
			return 3;
		}
	}
	/* set cw2015/cw2013 to use new battery info */
	ret = cw_read(REG_CONFIG, &reg_val);
	if(ret)
	{
		return 1;
	}
	reg_val |= CONFIG_UPDATE_FLG;   /* set UPDATE_FLAG */
	reg_val &= 0x07;                /* clear ATHD */
	reg_val |= ATHD;                /* set ATHD */
	ret = cw_write(REG_CONFIG, &reg_val);
	if(ret)
	{
		return 1;
	}
	/* reset */
	reset_val = MODE_NORMAL;
	reg_val = MODE_RESTART;
	ret = cw_write(REG_MODE, &reg_val);
	if(ret)
	{
		return 1;
	}
	CW_Delay10ms(1);  //delay 10ms      
	ret = cw_write(REG_MODE, &reset_val);
	if(ret)
	{
		return 1;
	} 
	CW_Delay10us(10);	
	return 0;
}


/*电量计初始化函数 每次开机后要执行
return 1 : i2c读写错， return 2 : 芯片处于sleep模式 return 3 : 写入的profile信息读出后与代码中的不一致 return 4 : 芯片启动后30s内读电量值一直异常*/
unsigned char cw_init(void)
{
	unsigned ret;
	unsigned char i;
	unsigned char reg_val = MODE_NORMAL;

	/* wake up cw2015/13 from sleep mode */
	ret = cw_write(REG_MODE, &reg_val);
	if(ret)
	{
		return 1;
	}

	/* check ATHD if not right */
	ret = cw_read(REG_CONFIG, &reg_val);
	if(ret)
	{
		return 1;
	}
	if((reg_val & 0xf8) != ATHD)
	{
		//"the new ATHD need set"
		reg_val &= 0x07;    /* clear ATHD */
		reg_val |= ATHD;    /* set ATHD */
		ret = cw_write(REG_CONFIG, &reg_val);
		if(ret)
		{
			return 1;
		}
	}
	
	/* check config_update_flag if not right */
	ret = cw_read(REG_CONFIG, &reg_val);
	if(ret)
	{
		return 1;
	}
	if(!(reg_val & CONFIG_UPDATE_FLG))
	{
		//"update flag for new battery info need set"
		ret = cw_update_config_info();
		
		if(ret)
		{
			return ret;
		}
	}
	else
	{
		for(i = 0; i < SIZE_BATINFO; i++)
		{ 
			ret = cw_read(REG_BATINFO +i, &reg_val);
			if(ret)
			{
				return 1;
			}
			
			/* debug_log("[cw_init]:REG[%02X] = %02X", REG_BATINFO +i, reg_val); */
			if(cw_bat_config_info[i] != reg_val)
			{
				break;
			}
		}
		
		if(i != SIZE_BATINFO)
		{
			reg_val = MODE_SLEEP;
			ret = cw_write(REG_MODE, &reg_val);
			if(ret)
			{
				return 1;
			}
			
			CW_Delay10ms(3);  //delay 30ms
			reg_val = MODE_NORMAL;
			ret = cw_write(REG_MODE, &reg_val);
			if(ret)
			{
				return 1;
			}
			
			//"update flag for new battery info need set"
			ret = cw_update_config_info();
			if(ret)
			{
				return ret;
			}
		}
	}
	/* check SOC if not eqaul 255 */
	for (i = 0; i < 30; i++) {
		CW_Delay10ms(10);//delay 100ms
		ret = cw_read(REG_SOC, &reg_val);
		if (ret)
			return 1;
		else if (reg_val <= 100) 
			break;		
    }
	
    if (i >=30){
        reg_val = MODE_SLEEP;
        ret = cw_write(REG_MODE, &reg_val);
        // "cw2015/cw2013 input unvalid power error_2\n";
        return 4;
    } 
	return 0;
}

uint8_t cw_por(void)
{
	uint8_t ret = 0;
	unsigned char reset_val = 0;
	reset_val = MODE_SLEEP;             
	ret = cw_write(REG_MODE, &reset_val);
	if (ret)
		return -1;
	CW_Delay10ms(3);  //delay 30ms
	
	reset_val = MODE_NORMAL;
	ret = cw_write(REG_MODE, &reset_val);
	if (ret)
		return -1;
	CW_Delay10us(10); //delay 100us
	
	ret = cw_init();
	if (ret) 
		return ret;
	return 0;
}

int cw_get_capacity(void)
{
	uint8_t ret = 0;
	unsigned char reg_val[2];
	unsigned char cw_capacity;
	unsigned int remainder = 0;
	unsigned int real_SOC = 0;
	unsigned int digit_SOC = 0;
	unsigned int UI_SOC = 0;

	ret = cw_read_word(REG_SOC, reg_val, 2);
	if(ret)
	{
		return -1;
	}
	
	real_SOC = reg_val[0];
	digit_SOC = reg_val[1];
	
	cw_capacity = real_SOC;
	
	/*假设ic出现问题，读取电量不在合理值范围内5次，重启ic。如果中间读到正确的值，那么5次的计数器清0，正确显示*/
	if (cw_capacity > 100) {
                // "get cw_capacity error; cw_capacity = %d\n"
        reset_loop++;
		if (reset_loop >5) { 
			ret = cw_por(); //por ic
			if(ret)
				return -1;
			reset_loop =0;               
		}                   
        return cw_bat.capacity;
    }else {
        reset_loop =0;
    }
	
	/*ic出错了，充了很久一直还是0%，一般我们用半个小时，那么重启下ic*/
	if((cw_bat.usb_online > 0) && (cw_capacity == 0))
	{		  
		allow_charger_always_zero++;
		if((allow_charger_always_zero >= BATTERY_DOWN_MIN_CHANGE_SLEEP) && (if_quickstart == 0))
		{
            ret = cw_por(); //por ic
			if(ret){
				return -1;
			}
			if_quickstart = 1;
			allow_charger_always_zero =0;
		}
	}
	else if((if_quickstart == 1)&&(cw_bat.usb_online == 0))
	{
		if_quickstart = 0;
	}

	//UI_SOC = ((real_SOC * 256 + digit_SOC) * 100)/ (UI_FULL * 256);
	//remainder = (((real_SOC * 256 + digit_SOC) * 100 * 100) / (UI_FULL * 256)) % 100;
	UI_SOC = real_SOC * 256 + digit_SOC;
 	UI_SOC = ((unsigned long)UI_SOC * 100)/ (UI_FULL * 256);
	remainder = real_SOC * 256 + digit_SOC;
 	remainder = (((unsigned long)remainder * 100 * 100) / (UI_FULL * 256)) % 100;
	
	/* debug_log("real_SOC = %d digit_SOC = %d ui_100 = %d UI_SOC = %d remainder = %d\n",
		real_SOC, digit_SOC, UI_FULL, UI_SOC, remainder); */
	
	/*aviod swing*/
	if(UI_SOC >= 100){
		UI_SOC = 100;
	}else if ((0 == UI_SOC) && (10 >= remainder)){
		UI_SOC = 0;
	}else{
		if((remainder > 80 || remainder < 20) && (UI_SOC >= (cw_bat.capacity - 1)) && (UI_SOC <= (cw_bat.capacity + 1)))
		{
			UI_SOC = cw_bat.capacity;
		}
	}

	return UI_SOC;
}

unsigned int cw_get_vol(void)
{
	uint8_t ret = 0;
	unsigned char get_ad_times = 0;
	unsigned char reg_val[2] = {0 , 0};
	unsigned long ad_value = 0;
	unsigned int ad_buff = 0;
	unsigned int ad_value_min = 0;
	unsigned int ad_value_max = 0;

	for(get_ad_times = 0; get_ad_times < 3; get_ad_times++)
	{
		ret = cw_read_word(REG_VCELL, &reg_val[0],2);
		if(ret)
		{
			return 1;
		}
		ad_buff = (reg_val[0] << 8) + reg_val[1];

		if(get_ad_times == 0)
		{
			ad_value_min = ad_buff;
			ad_value_max = ad_buff;
		}
		if(ad_buff < ad_value_min)
		{
			ad_value_min = ad_buff;
		}
		if(ad_buff > ad_value_max)
		{
			ad_value_max = ad_buff;
		}
		ad_value += ad_buff;
	}
	ad_value -= ad_value_min;
	ad_value -= ad_value_max;
	ad_value = ad_value  * 305 / 1000;
	return(ad_value);       //14位ADC转换值
}

void update_capacity(void)
{
	int cw_capacity;
	cw_capacity = cw_get_capacity();
	if((cw_capacity >= 0) && (cw_capacity <= 100) && (cw_bat.capacity != cw_capacity))
	{       
		cw_bat.capacity = cw_capacity;
	}
}


void update_vol(void)
{
	unsigned int cw_voltage;
	cw_voltage = cw_get_vol();
	if(cw_voltage == 1){
		//read voltage error
		cw_bat.voltage = cw_bat.voltage;
	}else if(cw_bat.voltage != cw_voltage)
	{
		cw_bat.voltage = cw_voltage;
	}
}

void update_usb_online(void)
{
	if(CHARGE == 1) 
	//这里请注意，这里是客户需要自己配置修改的地方
	//请修改代码保证DC插入时配置cw_bat.usb_online为 1，DC不在时配置cw_bat.usb_online为0
	{
		cw_bat.usb_online = 1;
	}else{
		cw_bat.usb_online = 0;
	}
}

/* 电池系统时钟(每1s调用1次) */
void cw_bat_tick_1s(void)
{
	update_usb_online();
	update_capacity();
	update_vol();
	/* debug_log("[CW2015]capacity = %d voltage = %d\n", cw_bat.capacity, cw_bat.voltage); */
}

static int cw_init_data(void)
{
	unsigned char reg_SOC[2];
	int real_SOC = 0;
	int digit_SOC = 0;
	int UI_SOC = 0;

	cw_read_word(REG_SOC, reg_SOC, 2);

	real_SOC = reg_SOC[0];
	digit_SOC = reg_SOC[1];
	//UI_SOC = ((real_SOC * 256 + digit_SOC) * 100)/ (UI_FULL*256);
	UI_SOC = real_SOC * 256 + digit_SOC;
 	UI_SOC = ((unsigned long)UI_SOC * 100)/ (UI_FULL * 256);
	/* debug_log("[cw_init_data]: real_SOC = %d  digit_SOC = %d\n", real_SOC, digit_SOC); */
	if(UI_SOC >= 100){
		UI_SOC = 100;
	}
	
	cw_bat.capacity = UI_SOC;	
	update_usb_online();
	update_vol();
#ifdef CW2015_GET_RRT
	update_time_to_empty();
#endif
	return 0;
}

unsigned char cw_bat_init(void)
{
	unsigned char ret;
	unsigned char loop = 0;
	//cw_bat_gpio_init();
	cw_por();
	ret = cw_init();
	while((loop++ < 3) && (ret != 0))
	{
		ret = cw_init();
	}
	cw_bat.alt = 0;
	cw_init_data();
	
	return ret;	
}

uint32_t Cw2015chbdGetVol(void)
{
    return cw_bat.voltage;
}

uint32_t Cw2015chbdGetBattery(void)
{
    return cw_bat.capacity;
}

uint8_t Cw2015chbdGetMode(void)
{
    uint8_t data;
    cw2015chbd_fun->i2c_read_data(CW2015CHBD_DEVICE_ADDR,REG_MODE,&data,1);
    return data;    
}

void Cw2015chbdSetMode(uint8_t data)
{
    cw2015chbd_fun->i2c_write_data(CW2015CHBD_DEVICE_ADDR,REG_MODE,&data,1);
}

void Cw2015chbdSetSleepMode(void)
{
    uint8_t data = 0xF0;
    cw2015chbd_fun->i2c_write_data(CW2015CHBD_DEVICE_ADDR,REG_MODE,&data,1);
}

void Cw2015chbdSetWakeMode(void)
{
    uint8_t data = 0x30;
    cw2015chbd_fun->i2c_write_data(CW2015CHBD_DEVICE_ADDR,REG_MODE,&data,1);   
}


void Cw2015chbdSetUsb(uint8_t value)
{
    CHARGE = value;
}



uint8_t Cw2015chbdInit(struct cw2015chbd_i2c_function *fun)
{
    cw2015chbd_fun = fun;
    if(cw_bat_init()!= CW2015CHBD_OK)
        return CW2015CHBD_ERROR;
    return CW2015CHBD_OK;
}