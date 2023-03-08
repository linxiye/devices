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

//�޼���дSPI FLASH
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ����
//��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
//CHECK OK
void _25qxx_Flash_Write_NoCheck(uint32_t addr,uint8_t* data,uint32_t len)
{
    uint16_t pageremain;
    pageremain = SPI_FLASH_PageSize - addr % SPI_FLASH_PageSize; //��ҳʣ����ֽ���
    if(len <= pageremain) pageremain = len;//������SPI_FLASH_PageSize���ֽ�
    while(1)
    {
        _25xx_Flash_Write_Page(addr, data, pageremain);
        if(len == pageremain)break;//д�������
        else //NumByteToWrite>pageremain
        {
            data += pageremain;
            addr += pageremain;

            len -= pageremain;           //��ȥ�Ѿ�д���˵��ֽ���
            if(len > SPI_FLASH_PageSize) pageremain = SPI_FLASH_PageSize; //һ�ο���д��SPI_FLASH_PageSize���ֽ�
            else pageremain = len;       //����SPI_FLASH_PageSize���ֽ���
        }
    };
}
//дSPI FLASH
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ú�������������!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)

void _25qxx_Flash_Write_Data(uint32_t addr,uint8_t* data,uint32_t len)
{
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;
    uint16_t i;

    secpos = addr / SPI_FLASH_SectorSize;//������ַ 0~511 for w25x16
    secoff = addr % SPI_FLASH_SectorSize;//�������ڵ�ƫ��
    secremain = SPI_FLASH_SectorSize - secoff;//����ʣ��ռ��С

    if(len <= secremain) secremain = len;//������SPI_FLASH_SectorSize���ֽ�
    while(1)
    {

        _25qxx_Flash_Read_Data(secpos*SPI_FLASH_SectorSize, dev.buff, SPI_FLASH_SectorSize);//������������������
        for(i=0; i<secremain; i++)//У������
        {
            if(dev.buff[secoff + i] != 0XFF)break;//��Ҫ����
        }
        if(i < secremain)//��Ҫ����
        {
            _25qxx_Flash_Erase_Sector(secpos);//�����������

            for(i=0; i<secremain; i++)       //����
            {
                dev.buff[i + secoff] = data[i];
            }
            _25qxx_Flash_Write_NoCheck(secpos*SPI_FLASH_SectorSize, dev.buff, SPI_FLASH_SectorSize);//д����������

        }else _25qxx_Flash_Write_NoCheck(addr, data, secremain);//д�Ѿ������˵�,ֱ��д������ʣ������.
        if(len == secremain)break;//д�������
        else//д��δ����
        {
            secpos++;//������ַ��1
            secoff=0;//ƫ��λ��Ϊ0

            data += secremain;  //ָ��ƫ��
            addr += secremain;//д��ַƫ��
            len -= secremain;              //�ֽ����ݼ�
            if(len > SPI_FLASH_SectorSize) secremain = SPI_FLASH_SectorSize;  //��һ����������д����
            else secremain = len;          //��һ����������д����
        }
    };
}


