#ifndef __74HC595_DEVICE_H
#define __74HC595_DEVICE_H

#include "stdint.h"
#include "board.h"
#include "rtthread.h"

struct hc595_config{
    uint8_t Diopin;
    uint8_t Rclpin;
    uint8_t Sclpin;
    uint8_t cascade;
};


#endif


