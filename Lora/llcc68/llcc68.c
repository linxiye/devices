#include "llcc68.h"
#include "string.h"
#include "drv_spi.h"
#include "drivers/pin.h"

struct llcc68_dev{
    rt_device_t spi_driver;
    struct llcc68_config *config;
};

const static char *TAG = "LLCC68";

static struct llcc68_dev llcc68_dev;


static struct drv_spi_config spi_config = {
    .cs_pin = -1,
    .flage = CPOL_0_CPHA_1,
    .data_direction = spi_direction_2line_fullduplex,
    .mode = spi_mode_master,
    .datasize = spi_datasize_8b,
    .nss = spi_nss_soft,
    .baudrate_prescaler = spi_baudrate_prescaler_256,
    .send_mode = spi_send_polling,
    .first_bit = spi_msb,
};

static void llcc68_set_reset(uint8_t value)
{
    rt_pin_write( llcc68_dev.config->Resetpin,value);
}

static void llcc68_set_nss(uint8_t value)
{
    rt_pin_write( llcc68_dev.config->Nsspin,value);
}

static uint8_t llcc68_get_busy(void)
{
    return rt_pin_read(llcc68_dev.config->Busypin);
}

static uint8_t llcc68_get_dio1(void)
{
    return rt_pin_read(llcc68_dev.config->Dio1pin);
}

uint8_t SpiSendReceive(uint8_t TxData)
{
    uint8_t value = TxData;
    rt_device_read(llcc68_dev.spi_driver, 0, &value, sizeof(TxData));
    return value;
}

void llcc68_delay_ms(uint32_t ms)
{
    rt_thread_mdelay(ms);
}

static struct sx126x_board_dev board_dev = {
    .llcc68_set_reset = llcc68_set_reset,
    .llcc68_set_nss = llcc68_set_nss,
    .llcc68_get_busy = llcc68_get_busy,
    .llcc68_get_dio1 = llcc68_get_dio1,
    .spiinout = SpiSendReceive,
    .delayms = llcc68_delay_ms,
};

rt_err_t llcc68_device_init(rt_device_t dev)
{
    rt_err_t ret;
    if(llcc68_dev.config->spidrv == NULL){
        RT_LOGE(TAG,"No Appoint SPI!!");
        return RT_ERROR;
    }

    llcc68_dev.spi_driver = rt_device_find(llcc68_dev.config->spidrv);
    if(llcc68_dev.spi_driver == NULL){
        RT_LOGE(TAG,"No Find SPI Driver!!");
        return RT_ERROR;
    }

    RT_LOGI(TAG,"LLCC68 Find SPI Driver Success!!");
    ret = rt_device_control(llcc68_dev.spi_driver,RT_DEVICE_CTRL_CONFIG,&spi_config);
    if(ret != RT_EOK)
        return ret;

    ret = rt_device_init(llcc68_dev.spi_driver);
    if(ret != RT_EOK)
        return ret;

    ret = rt_device_open(llcc68_dev.spi_driver, RT_DEVICE_OFLAG_RDWR);
    if(ret != RT_EOK)
        return ret;

    RT_LOGI(TAG,"LLCC68 SPI Driver Init Success!!");

    rt_pin_mode(llcc68_dev.config->Nsspin, PIN_MODE_OUTPUT);
    rt_pin_mode(llcc68_dev.config->Resetpin, PIN_MODE_OUTPUT);
    rt_pin_mode(llcc68_dev.config->Busypin, PIN_MODE_INPUT);
    rt_pin_mode(llcc68_dev.config->Dio1pin, PIN_MODE_INPUT);

    return ret;
}


rt_err_t llcc68_device_open(rt_device_t dev, rt_uint16_t oflag)
{
    SX126xBoardRegister(&board_dev);

    Radio.Init(&llcc68_dev.config->RadioEvents);
    Radio.SetChannel( llcc68_dev.config->Rf_Frequency );
    if(llcc68_dev.config->Mode == Lora_Mode){
        Radio.SetTxConfig( llcc68_dev.config->Mode, llcc68_dev.config->lora_config.Tx_Power_dBm,
                                           0, llcc68_dev.config->lora_config.BandWidth,
                                           llcc68_dev.config->lora_config.Sf, llcc68_dev.config->lora_config.Cr,
                                           llcc68_dev.config->lora_config.Preamble_Length, llcc68_dev.config->lora_config.Lora_Fix_Length_Payload,
                                           true, 0, 0, llcc68_dev.config->lora_config.Lora_Iq_Inversion, 3000 );

        Radio.SetRxConfig( llcc68_dev.config->Mode, llcc68_dev.config->lora_config.BandWidth,
                                        llcc68_dev.config->lora_config.Sf,llcc68_dev.config->lora_config.Cr,
                                        0, llcc68_dev.config->lora_config.Preamble_Length,
                                        llcc68_dev.config->lora_config.symbTimeout, llcc68_dev.config->lora_config.Lora_Fix_Length_Payload,
                                        0, true, 0, 0, llcc68_dev.config->lora_config.Lora_Iq_Inversion, false );
        RT_LOGI(TAG,"LLCC68 Run Lora Mode");
    }else{
        RT_LOGW(TAG,"There is no FSK mode now");
    }
    RT_LOGI(TAG,"LLCC68 Init Success");
    RT_LOGI(TAG,"LLCC68 Rf_Frequency:%dHz",llcc68_dev.config->Rf_Frequency);

    RT_LOGI(TAG,"LLCC68 Open Success");
    Radio.Standby();
    Radio.Rx( llcc68_dev.config->rxtimeout );
    return RT_EOK;
}

rt_err_t llcc68_device_close(rt_device_t dev)
{
    RT_LOGI(TAG,"LLCC68 Close Success");
    return RT_EOK;
}

rt_size_t llcc68_device_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    Radio.IrqProcess();
    return RT_EOK;
}


rt_size_t llcc68_device_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    Radio.Send((uint8_t *)buffer, size);
    return RT_EOK;
}

rt_err_t llcc68_device_control(rt_device_t dev, int cmd, void *args)
{
    if(cmd == RT_DEVICE_CTRL_CONFIG){
        llcc68_dev.config = args;
        RT_LOGI(TAG,"LLCC68 Config Success!!");
    }
    return RT_EOK;
}

static struct rt_device llcc68_device = {
    .type = RT_Device_Class_Char,
    .rx_indicate = RT_NULL,
    .tx_complete = RT_NULL,
    .init = llcc68_device_init,
    .open = llcc68_device_open,
    .close = llcc68_device_close,
    .read = llcc68_device_read,
    .write = llcc68_device_write,
    .control = llcc68_device_control,
    .user_data = NULL,
};

int llcc68_init(void)
{
    rt_err_t ret = RT_EOK;
    ret = rt_device_register(&llcc68_device, "llcc68", RT_DEVICE_FLAG_RDWR);
    if(ret == RT_ERROR)
        return RT_ERROR;
    return ret;
}
INIT_DEVICE_EXPORT(llcc68_init);
