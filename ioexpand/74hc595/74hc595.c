#include "74hc595.h"

static struct hc595_function *hc595_fun;


void LedShow(uint8_t value)
{
	for(int i = 0;i<8;i++)
	{
		if(value&0x80)
			hc595_fun->SetDioValue(1);
		else
			hc595_fun->SetDioValue(0);
		value <<= 1;
		hc595_fun->SetSclkValue(0);
		hc595_fun->SetSclkValue(1);
	}
}

void Hc595Set(uint8_t *value)
{
    for(int i = 0;i < hc595_fun->cascade;i++)
    {
        LedShow(value[i]);
    }
	hc595_fun->SetRclkValue(0);
	hc595_fun->SetRclkValue(1);
}

void Hc595Init(struct hc595_function *fun)
{
	hc595_fun = fun;
	/* 
	 * 初始化不输出电平
	 */
	hc595_fun->SetDioValue(1);
	hc595_fun->SetSclkValue(1);
	hc595_fun->SetRclkValue(0);
	hc595_fun->SetRclkValue(1);
}
