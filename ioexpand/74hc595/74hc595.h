#ifndef __74HC595_H
#define __74HC595_H

#include "stdint.h"

struct hc595_function{
	void (*SetDioValue)(uint8_t value);
	void (*SetRclkValue)(uint8_t value);
	void (*SetSclkValue)(uint8_t value);
	uint8_t cascade;
};

void Hc595Set(uint8_t *value);
void Hc595Init(struct hc595_function *fun);
#endif

