/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: SX126x driver specific target board functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "radio.h"
#include "sx126x.h"
#include "sx126x-board.h"
#include "rtthread.h"
#include "drv_log.h"

const static char *TAG = "Lora";

static struct sx126x_board_dev *board_dev;

uint8_t SpiInOut(uint8_t TxData)
{
    return board_dev->spiinout(TxData);
}

void SX126xReset( void )
{
    rt_thread_mdelay( 10 );
    board_dev->llcc68_set_reset(0);
    rt_thread_mdelay( 20 );
    board_dev->llcc68_set_reset(1);
    rt_thread_mdelay( 10 );
    RT_LOGI(TAG,"Lora Reset!!");
}


void SX126xWaitOnBusy( void )
{
   while(board_dev->llcc68_get_busy() == 1);
}

uint8_t Read_DIO1(void)
{
    return board_dev->llcc68_get_dio1();
}

void SX126xWakeup( void )
{
    board_dev->llcc68_set_nss(0);
   
    SpiInOut(RADIO_GET_STATUS);
    SpiInOut(0);
    board_dev->llcc68_set_nss(1);

    // Wait for chip to be ready.
    SX126xWaitOnBusy( );
}

void SX126xWriteCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{

    SX126xCheckDeviceReady( );

    board_dev->llcc68_set_nss(0);

    SpiInOut(( uint8_t )command );

    for( uint16_t i = 0; i < size; i++ )
    {
        SpiInOut(buffer[i] );
    }

    board_dev->llcc68_set_nss(1);
    
    if( command != RADIO_SET_SLEEP )
    {
        SX126xWaitOnBusy( );
    }
}

void SX126xReadCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );

    board_dev->llcc68_set_nss(0);

    SpiInOut(( uint8_t )command );
    SpiInOut(0x00 );
    for( uint16_t i = 0; i < size; i++ )
    {
        buffer[i] = SpiInOut(0);
    }

    board_dev->llcc68_set_nss(1);

    SX126xWaitOnBusy( );
}

void SX126xWriteRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );

    board_dev->llcc68_set_nss(0);
    
    SpiInOut(RADIO_WRITE_REGISTER );
    SpiInOut(( address & 0xFF00 ) >> 8 );
    SpiInOut( address & 0x00FF );
    
    for( uint16_t i = 0; i < size; i++ )
    {
        SpiInOut(buffer[i] );
    }


    board_dev->llcc68_set_nss(1);

    SX126xWaitOnBusy( );
}

void SX126xWriteRegister( uint16_t address, uint8_t value )
{
    SX126xWriteRegisters( address, &value, 1 );
}

void SX126xReadRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );

    board_dev->llcc68_set_nss(0);

    SpiInOut(RADIO_READ_REGISTER );
    SpiInOut(( address & 0xFF00 ) >> 8 );
    SpiInOut( address & 0x00FF );
    SpiInOut( 0 );
    for( uint16_t i = 0; i < size; i++ )
    {
        buffer[i] = SpiInOut(0 );
    }

    board_dev->llcc68_set_nss(1);

    SX126xWaitOnBusy( );
}

uint8_t SX126xReadRegister( uint16_t address )
{
    uint8_t data;
    SX126xReadRegisters( address, &data, 1 );
    return data;
}

void SX126xWriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    SX126xCheckDeviceReady( );

    board_dev->llcc68_set_nss(0);
    
    SpiInOut( RADIO_WRITE_BUFFER );
    SpiInOut( offset );
    for( uint16_t i = 0; i < size; i++ )
    {
        SpiInOut( buffer[i] );
    }

    board_dev->llcc68_set_nss(1);

    SX126xWaitOnBusy( );
}

void SX126xReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    SX126xCheckDeviceReady( );

    board_dev->llcc68_set_nss(0);

    SpiInOut(  RADIO_READ_BUFFER );
    SpiInOut(  offset );
    SpiInOut(  0 );
    for( uint16_t i = 0; i < size; i++ )
    {
        buffer[i] = SpiInOut( 0 );
    }

    board_dev->llcc68_set_nss(1);
    
    SX126xWaitOnBusy( );
}

void SX126xSetRfTxPower( int8_t power )
{
    SX126xSetTxParams( power, RADIO_RAMP_40_US );
}

uint8_t SX126xGetPaSelect( uint32_t channel )
{
//    if( GpioRead( &DeviceSel ) == 1 )
//    {
//        return SX1261;
//    }
//    else
//    {
//        return SX1262;
//    }
  
  return SX1262;
}

void SX126xAntSwOn( void )
{
    //GpioInit( &AntPow, ANT_SWITCH_POWER, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
}

void SX126xAntSwOff( void )
{
   // GpioInit( &AntPow, ANT_SWITCH_POWER, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

bool SX126xCheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}

void SX126xDelayMs(uint32_t ms)
{
    board_dev->delayms(ms);
}


void SX126xBoardRegister(struct sx126x_board_dev *dev)
{
    board_dev = dev;
}


