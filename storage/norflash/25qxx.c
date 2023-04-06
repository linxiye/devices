#include "25qxx.h"
#include "stdlib.h"
#include "drv_log.h"

struct _25qxx_device {
    struct _25qxx_function *fun;
    uint8_t *buff;
};


static struct _25qxx_device dev;

void _25qxx_Init(struct _25qxx_function *p)
{
    dev.fun = p;
    dev.buff = malloc(SPI_FLASH_SectorSize);
    dev.fun->_25qxx_spi_cs(1);
}

uint32_t _25qxx_ReadID(void)
{
    uint8_t data[4] = {CMD_FLASH_JEDEC_ID,0xFF,0xFF,0xFF};
    dev.fun->_25qxx_spi_cs(0);
    dev.fun->_25qxx_spi_rw(data,sizeof(data));
    dev.fun->_25qxx_spi_cs(1);
    return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

uint32_t _25qxx_Flash_Sector_Count(void)
{
    uint32_t count = 0;
    uint32_t Flash_ID;
    /* Read FLASH ID */
    Flash_ID = _25qxx_ReadID( );
    Flash_ID &= 0xFFFF;

    switch( Flash_ID )
    {
        /* W25XXX */
        case _25X10_FLASH_ID:                                                   /* 0xEF3011-----1M bit */
            count = 1;
            break;

        case _25X20_FLASH_ID:                                                   /* 0xEF3012-----2M bit */
            count = 2;
            break;

        case _25X40_FLASH_ID:                                                   /* 0xEF3013-----4M bit */
            count = 4;
            break;

        case _25X80_FLASH_ID:                                                   /* 0xEF4014-----8M bit */
            count = 8;
            break;

        case _25Q16_FLASH_ID1:                                                  /* 0xEF3015-----16M bit */
        case _25Q16_FLASH_ID2:                                                  /* 0xEF4015-----16M bit */
            count = 16;
            break;

        case _25Q32_FLASH_ID1:                                                  /* 0xEF4016-----32M bit */
        case _25Q32_FLASH_ID2:                                                  /* 0xEF6016-----32M bit */
            count = 32;
            break;

        case _25Q64_FLASH_ID1:                                                  /* 0xEF4017-----64M bit */
        case _25Q64_FLASH_ID2:                                                  /* 0xEF6017-----64M bit */
            count = 64;
            break;

        case _25Q128_FLASH_ID1:                                                 /* 0xEF4018-----128M bit */
        case _25Q128_FLASH_ID2:                                                 /* 0xEF6018-----128M bit */
            count = 128;
            break;

        case _25Q256_FLASH_ID1:                                                 /* 0xEF4019-----256M bit */
        case _25Q256_FLASH_ID2:                                                 /* 0xEF6019-----256M bit */
            count = 256;
            break;
        default:
                count = 0x00;
            break;
    }

    count = count * 2;//count * 1024 * 8 / SPI_FLASH_SectorSize;
    return count;
}

void _25qxx_Write_Enable(void)
{
    uint8_t data = CMD_FLASH_WREN;
    dev.fun->_25qxx_spi_cs(0);
    dev.fun->_25qxx_spi_rw(&data,sizeof(data));
    dev.fun->_25qxx_spi_cs(1);
}


void _25qxx_Write_Disable(void)
{
    uint8_t data = CMD_FLASH_WRDI;
    dev.fun->_25qxx_spi_cs(0);
    dev.fun->_25qxx_spi_rw(&data,sizeof(data));
    dev.fun->_25qxx_spi_cs(1);
}


uint8_t _25qxx_Flash_ReadStatusReg(void)
{
    uint8_t  data[2] = {CMD_FLASH_RDSR,0xFF};
    dev.fun->_25qxx_spi_cs(0);
    dev.fun->_25qxx_spi_rw(data,sizeof(data));
    dev.fun->_25qxx_spi_cs(1);
    return data[1];
}

void _25qxx_Flash_Wait_SR(void)
{
    while ((_25qxx_Flash_ReadStatusReg()&0x01)==0x01);
}

void _25qxx_Flash_Erase_Sector(uint32_t address)
{
    uint8_t data[4];
    _25qxx_Write_Enable( );
    dev.fun->_25qxx_spi_cs(0);

    data[0] = CMD_FLASH_SECTOR_ERASE;
    data[1] = (uint8_t)( address >> 16 ) ;
    data[2] = (uint8_t)( address >> 8 ) ;
    data[3] = (uint8_t)address ;
    dev.fun->_25qxx_spi_rw(data,sizeof(data));
    dev.fun->_25qxx_spi_cs(1);
    _25qxx_Flash_Wait_SR();
}

void _25qxx_Flash_Read_Data(uint32_t addr,uint8_t *data,uint32_t len)
{
    uint8_t buff[4];
    dev.fun->_25qxx_spi_cs(0);
    buff[0] = CMD_FLASH_READ;
    buff[1] = (uint8_t)( addr >> 16 ) ;
    buff[2] = (uint8_t)( addr >> 8 ) ;
    buff[3] = (uint8_t)addr ;
    dev.fun->_25qxx_spi_rw(buff,sizeof(buff));
    dev.fun->_25qxx_spi_rw(data,len);
    dev.fun->_25qxx_spi_cs(1);

}

void _25xx_Flash_Write_Page(uint32_t addr,uint8_t *data, uint32_t len)
{

    uint8_t buff[4];
    _25qxx_Write_Enable();
    dev.fun->_25qxx_spi_cs(0);
    buff[0] = CMD_FLASH_BYTE_PROG;
    buff[1] = (uint8_t)( addr >> 16 );
    buff[2] = (uint8_t)( addr >> 8 );
    buff[3] = (uint8_t)addr;
    dev.fun->_25qxx_spi_rw(buff, sizeof(buff));
    dev.fun->_25qxx_spi_rw(data,len);
    dev.fun->_25qxx_spi_cs(1);
    _25qxx_Flash_Wait_SR();
}

//无检验写SPI FLASH
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void _25qxx_Flash_Write_NoCheck(uint32_t addr,uint8_t* data,uint32_t len)
{
    uint16_t pageremain;
    pageremain = SPI_FLASH_PageSize - addr % SPI_FLASH_PageSize; //单页剩余的字节数
    if(len <= pageremain) pageremain = len;//不大于SPI_FLASH_PageSize个字节
    while(1)
    {
        _25xx_Flash_Write_Page(addr, data, pageremain);
        if(len == pageremain)break;//写入结束了
        else //NumByteToWrite>pageremain
        {
            data += pageremain;
            addr += pageremain;

            len -= pageremain;           //减去已经写入了的字节数
            if(len > SPI_FLASH_PageSize) pageremain = SPI_FLASH_PageSize; //一次可以写入SPI_FLASH_PageSize个字节
            else pageremain = len;       //不够SPI_FLASH_PageSize个字节了
        }
    };
}
//写SPI FLASH
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)

void _25qxx_Flash_Write_Data(uint32_t addr,uint8_t* data,uint32_t len)
{
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;
    uint16_t i;

    secpos = addr / SPI_FLASH_SectorSize;//扇区地址 0~511 for w25x16
    secoff = addr % SPI_FLASH_SectorSize;//在扇区内的偏移
    secremain = SPI_FLASH_SectorSize - secoff;//扇区剩余空间大小

    if(len <= secremain) secremain = len;//不大于SPI_FLASH_SectorSize个字节
    while(1)
    {

        _25qxx_Flash_Read_Data(secpos*SPI_FLASH_SectorSize, dev.buff, SPI_FLASH_SectorSize);//读出整个扇区的内容
        for(i=0; i<secremain; i++)//校验数据
        {
            if(dev.buff[secoff + i] != 0XFF)break;//需要擦除
        }
        if(i < secremain)//需要擦除
        {
            _25qxx_Flash_Erase_Sector(secpos);//擦除这个扇区

            for(i=0; i<secremain; i++)       //复制
            {
                dev.buff[i + secoff] = data[i];
            }
            _25qxx_Flash_Write_NoCheck(secpos*SPI_FLASH_SectorSize, dev.buff, SPI_FLASH_SectorSize);//写入整个扇区

        }else _25qxx_Flash_Write_NoCheck(addr, data, secremain);//写已经擦除了的,直接写入扇区剩余区间.
        if(len == secremain)break;//写入结束了
        else//写入未结束
        {
            secpos++;//扇区地址增1
            secoff=0;//偏移位置为0

            data += secremain;  //指针偏移
            addr += secremain;//写地址偏移
            len -= secremain;              //字节数递减
            if(len > SPI_FLASH_SectorSize) secremain = SPI_FLASH_SectorSize;  //下一个扇区还是写不完
            else secremain = len;          //下一个扇区可以写完了
        }
    };
}


