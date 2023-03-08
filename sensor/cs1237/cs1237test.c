#include "cs1237device.h"

struct cs1237test_dev{
    rt_device_t device;
    struct rt_thread thread;
    uint8_t stack[512];
};

static struct cs1237test_dev dev;
const static char *TAG = "CS1237APP";

static struct cs1237_dev_config config = {
    .Sclpin = 19,
    .Sdapin = 18,
    .config = {
        .ch = adc_ch,       //ADC通道
        .pga = pga1,        //pga1倍
        .refo = refo_on,    //打开使用内部基准电压
        .speed = adc40Hz,   //ADC采集频率
    },
};


void cs1237test_task(void *argv)
{
    rt_err_t ret;
    dev.device = rt_device_find("cs1237");
    if(dev.device == NULL){
        RT_LOGE(TAG,"CS1237 Device can't find");
        goto cs1237test_initerr;
    }

    ret = rt_device_control(dev.device,RT_DEVICE_CTRL_CONFIG,&config);
    if(ret != RT_EOK){
        RT_LOGE(TAG,"CS1237 Device config err");
        goto cs1237test_initerr;
    }

    ret = rt_device_init(dev.device);
    if(ret != RT_EOK){
        RT_LOGE(TAG,"CS1237 Device init err");
        goto cs1237test_initerr;
    }

    ret = rt_device_open(dev.device,RT_DEVICE_OFLAG_RDWR);
    if(ret != RT_EOK){
        RT_LOGE(TAG,"CS1237 Device open err");
        goto cs1237test_initerr;
    }
    int value;

    while(1)
    {
        rt_device_read(dev.device, 0, &value, sizeof(value));
        RT_LOGI(TAG,"cs1237 value:%d",value);
        rt_thread_mdelay(1000);
    }

cs1237test_initerr:
    while(1)
    {
        rt_thread_mdelay(10);
    }
}


int cs1237test_app(void)
{
    rt_err_t ret;
    ret = rt_thread_init(&dev.thread,
                            "cs1237",
                            cs1237test_task,
                            NULL,
                            dev.stack,
                            sizeof(dev.stack),
                            14, 20);
    if(ret != RT_EOK){
        RT_LOGE(TAG,"CS1237 Thread Init Error!!");
        return RT_ERROR;
    }
    rt_thread_startup(&dev.thread);
    return RT_EOK;
}
//INIT_APP_EXPORT(cs1237test_app);
