#ifndef __CS1237_DEVICE_H
#define __CS1237_DEVICE_H

#include "board.h"
#include "rtthread.h"
#include "stdint.h"
#include "cs1237.h"

struct cs1237_dev_config{
    uint8_t Sclpin;
    uint8_t Sdapin;
    struct cs1237_config config;
};

#endif
