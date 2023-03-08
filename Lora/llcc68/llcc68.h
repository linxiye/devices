#ifndef __LLCC68_H
#define __LLCC68_H

#include "board.h"
#include "rtthread.h"
#include "stdbool.h"
#include "sx126x-board.h"

/*
 * Lora 发送波特率
 */
typedef enum lora_band_t{
    Lora_BW_125 = 0x00,
    Lora_BW_250 = 0x01,
    Lora_BW_500 = 0x02,
    Lora_band_reserved = 0x03,
}Lora_band;

typedef enum Lora_sf_t{
    Lora_sf_5 = 0x05,
    Lora_sf_6 = 0x06,
    Lora_sf_7 = 0x07,
    Lora_sf_8 = 0x08,
    Lora_sf_9 = 0x09,
    Lora_sf_10 = 0x0A,
    Lora_sf_11 = 0x0B,
}Lora_sf;

typedef enum Lora_cr_t{
    Lora_cr_4_5 = 0x01,
    Lora_cr_4_6 = 0x02,
    Lora_cr_4_7 = 0x03,
    Lora_cr_4_8 = 0x04,
}Lora_cr;

typedef enum Lora_Preamble_Length_t{
    Preamble_detector_length_off = 0x00,
    Preamble_detector_length_8_bits = 0x08,
    Preamble_detector_length_16_bits = 0x10,
    Preamble_detector_length_24_bits = 0x18,
    Preamble_detector_length_32_bits = 0x20,
}Lora_Preamble_Length;

typedef enum LLCC68_Run_Mode_t{
    Fsk_Mode = 0x00,
    Lora_Mode = 0x01,
}LLCC68_Run_Mode;

struct llcc68_lora_config{
    Lora_band BandWidth;           /* 发送波特率 */
    Lora_sf   Sf;
    Lora_cr   Cr;
    Lora_Preamble_Length Preamble_Length;
    bool Lora_Fix_Length_Payload;
    bool Lora_Iq_Inversion;
    uint8_t symbTimeout;
    uint8_t Tx_Power_dBm;
};

struct llcc68_config{
   char *spidrv;                /* spi 外设选择 */
   uint32_t Rf_Frequency;       /* Lora 频段
                                   [430000000 - 440000000]Hz
                                   [470000000 - 510000000]Hz
                                   [779000000 - 787000000]Hz
                                   [863000000 - 870000000]Hz
                                   [902000000 - 928000000]Hz
                                   [150MHZ - 960MHZ]
                                 */
   struct llcc68_lora_config lora_config;
   LLCC68_Run_Mode Mode;
   RadioEvents_t RadioEvents;
   uint8_t Busypin;
   uint8_t Nsspin;
   uint8_t Resetpin;
   uint8_t Dio1pin;
   uint16_t rxtimeout;
};

#endif
