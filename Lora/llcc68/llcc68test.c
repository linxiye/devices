#include "llcc68.h"
#include "crc.h"
#include "string.h"
#include "drivers/pin.h"

#define LLCC68_BUSY     39
#define LLCC68_RESET    18
#define LLCC68_NSS      14
#define LLCC68_DIO1     19

#define LLCC68_LED      45
#define RX_TIMEOUT_VALUE 200
static uint8_t TX_Buffer[64];
static uint8_t RX_Buffer[64];
static rt_device_t device;
const static char *TAG = "llcc68test";

void OnTxDone( void )
{
    Radio.Standby();
    Radio.Rx( RX_TIMEOUT_VALUE ); //进入接收
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    static uint8_t led = 0;
    led = ~led;
    rt_pin_write(LLCC68_LED, led);
    memcpy( RX_Buffer, payload, size );
    RT_LOGI(TAG,"%s rssi:%d snr:%d",RX_Buffer,rssi,snr);
    Radio.Standby();

    TX_Buffer[0] = 'P';
    TX_Buffer[1] = 'I';
    TX_Buffer[2] = 'N';
    TX_Buffer[3] = 'G';

    uint16_t crc_value=RadioComputeCRC(TX_Buffer,4,CRC_TYPE_IBM);//计算得出要发送数据包CRC值
    TX_Buffer[4]=crc_value>>8;
    TX_Buffer[5]=crc_value;
    rt_device_write(device, 0, TX_Buffer, 6);

}

void OnTxTimeout( void )
{
    RT_LOGI(TAG, "TxTIMEOUT");
}

void OnRxTimeout( void )
{
    Radio.Standby();
    TX_Buffer[0] = 'P';
    TX_Buffer[1] = 'I';
    TX_Buffer[2] = 'N';
    TX_Buffer[3] = 'G';

    uint16_t crc_value = RadioComputeCRC(TX_Buffer,4,CRC_TYPE_IBM);//计算得出要发送数据包CRC值
    TX_Buffer[4]=crc_value>>8;
    TX_Buffer[5]=crc_value;
    rt_device_write(device, 0, TX_Buffer, 6);
    RT_LOGI(TAG, "RxTIMEOUT");
}

void OnRxError( void )
{

    Radio.Standby();
    TX_Buffer[0] = 'P';
    TX_Buffer[1] = 'I';
    TX_Buffer[2] = 'N';
    TX_Buffer[3] = 'G';

    uint16_t crc_value=RadioComputeCRC(TX_Buffer,4,CRC_TYPE_IBM);//计算得出要发送数据包CRC值
    TX_Buffer[4]=crc_value>>8;
    TX_Buffer[5]=crc_value;

    rt_device_write(device, 0, TX_Buffer, 6);
}

static struct llcc68_config config = {
    .spidrv = "spi1",
    .Mode = Lora_Mode,
    .Rf_Frequency = 434000000,
    .lora_config = {
        .BandWidth = Lora_BW_125,
        .Sf = Lora_sf_7,
        .Cr = Lora_cr_4_5,
        .Preamble_Length = Preamble_detector_length_8_bits,
        .Lora_Fix_Length_Payload = false,
        .Lora_Iq_Inversion = false,
        .symbTimeout = 0,
        .Tx_Power_dBm = 22,
    },
    .RadioEvents = {
        .TxDone = OnTxDone,
        .RxDone = OnRxDone,
        .TxTimeout = OnTxTimeout,
        .RxTimeout = OnRxTimeout,
        .RxError = OnRxError,
    },
    .Busypin = LLCC68_BUSY,
    .Dio1pin = LLCC68_DIO1,
    .Nsspin = LLCC68_NSS,
    .Resetpin = LLCC68_RESET,
};

void llcc68test_task(void *argv)
{
    rt_err_t ret;

    rt_pin_mode(LLCC68_LED, PIN_MODE_OUTPUT);
    device = rt_device_find("llcc68");
    if(device == NULL){
        RT_LOGE(TAG,"LLCC68 Device can't find");
        goto llcc68test_initerr;
    }

    ret = rt_device_control(device,RT_DEVICE_CTRL_CONFIG,&config);
    if(ret != RT_EOK){
        RT_LOGE(TAG,"LLCC68 Device config err");
        goto llcc68test_initerr;
    }

    ret = rt_device_init(device);
    if(ret != RT_EOK){
        RT_LOGE(TAG,"LLCC68 Device init err");
        goto llcc68test_initerr;
    }

    ret = rt_device_open(device,RT_DEVICE_OFLAG_RDWR);
    if(ret != RT_EOK){
        RT_LOGE(TAG,"LLCC68 Device open err");
        goto llcc68test_initerr;
    }

    OnRxTimeout();
    while(1)
    {
        rt_device_read(device, 0, &ret, sizeof(rt_err_t));
        rt_thread_mdelay(10);
    }

llcc68test_initerr:
    while(1){
        rt_thread_mdelay(10);
    }
}

struct rt_thread llcc68_thread;
uint8_t llcc68_stack[512];

int llcc68test_app(void)
{
    rt_err_t ret;
    ret = rt_thread_init(&llcc68_thread,
                            "LoraTH",
                            llcc68test_task,
                            NULL,
                            llcc68_stack,
                            sizeof(llcc68_stack),
                            12, 20);
    if(ret != RT_EOK){
        RT_LOGE(TAG,"LLCC68 APP Thread Init Error!!");
        return RT_ERROR;
    }
    rt_thread_startup(&llcc68_thread);
    return RT_EOK;
}
//INIT_APP_EXPORT(llcc68test_app);
