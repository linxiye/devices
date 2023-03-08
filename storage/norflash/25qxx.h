#ifndef __25QXX_H
#define __25QXX_H

#include "stdint.h"

/******************************************************************************/
/* SPI Serial Flash OPERATION INSTRUCTIONS */
#define CMD_FLASH_READ             0x03                                         /* Read Memory at 25 MHz */
#define CMD_FLASH_SECTOR_ERASE     0x20                                         /* Erase 4 KByte of memory array */
#define CMD_FLASH_BYTE_PROG        0x02                                         /* To Program One Data Byte */
#define CMD_FLASH_RDSR             0x05                                         /* Read-Status-Register */
#define CMD_FLASH_EWSR             0x50                                         /* Enable-Write-Status-Register */
#define CMD_FLASH_WREN             0x06                                         /* Write-Enable */
#define CMD_FLASH_WRDI             0x04                                         /* Write-Disable */
#define CMD_FLASH_JEDEC_ID         0x9F                                         /* JEDEC ID read */

/******************************************************************************/
#define DEF_DUMMY_BYTE             0xFF

/* FLASH Parameter Definition */
#define SPI_FLASH_SectorSize       4096
#define SPI_FLASH_PageSize         256
#define SPI_FLASH_PerWritePageSize 256


/* SPI FLASH Type Define */
/* _25QXX */
#define _25X10_FLASH_ID            0x3011                                     /* 1M bit */
#define _25X20_FLASH_ID            0x3012                                     /* 2M bit */
#define _25X40_FLASH_ID            0x3013                                     /* 4M bit */
#define _25X80_FLASH_ID            0x4014                                     /* 8M bit */
#define _25Q16_FLASH_ID1           0x3015                                     /* 16M bit */
#define _25Q16_FLASH_ID2           0x4015                                     /* 16M bit */
#define _25Q32_FLASH_ID1           0x4016                                     /* 32M bit */
#define _25Q32_FLASH_ID2           0x6016                                     /* 32M bit */
#define _25Q64_FLASH_ID1           0x4017                                     /* 64M bit */
#define _25Q64_FLASH_ID2           0x6017                                     /* 64M bit */
#define _25Q128_FLASH_ID1          0x4018                                     /* 128M bit */
#define _25Q128_FLASH_ID2          0x6018                                     /* 128M bit */
#define _25Q256_FLASH_ID1          0x4019                                     /* 256M bit */
#define _25Q256_FLASH_ID2          0x6019                                     /* 256M bit */

struct _25qxx_function{
    void (*_25qxx_spi_cs)(uint8_t value);
    void (*_25qxx_spi_rw)(uint8_t *data,uint32_t len);
};

void _25qxx_Init(struct _25qxx_function *p);
uint32_t _25qxx_ReadID(void);
uint32_t _25qxx_Flash_Sector_Count( void );
void _25qxx_Flash_Erase_Sector(uint32_t address);
void _25qxx_Flash_Read_Data(uint32_t addr,uint8_t *data,uint32_t len);
void _25xx_Flash_Write_Page(uint32_t addr,uint8_t *data, uint32_t len);
void _25qxx_Flash_Write_NoCheck(uint32_t addr,uint8_t* data,uint32_t len);
void _25qxx_Flash_Write_Data(uint32_t addr,uint8_t* data,uint32_t len);

#endif

