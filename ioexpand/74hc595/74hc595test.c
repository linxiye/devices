#include "74hc595device.h"

const static char *TAG = "hc595test";

struct hc595_dev{
    rt_device_t device;
    struct rt_thread hc595_thread;
    uint8_t hc595_stack[512];
};

static struct hc595_dev dev;

static struct hc595_config config = {
    .Diopin = 45,
    .Rclpin = 46,
    .Sclpin = 2,
    .cascade = 1,
};

void hc595test_task(void *argv)
{
    rt_err_t ret;
    dev.device = rt_device_find("hc595");
    if(dev.device == NULL){
        RT_LOGE(TAG,"HC595 Device can't find");
        goto hc595test_initerr;
    }

    ret = rt_device_control(dev.device,RT_DEVICE_CTRL_CONFIG,&config);
    if(ret != RT_EOK){
        RT_LOGE(TAG,"HC595 Device config err");
        goto hc595test_initerr;
    }

    ret = rt_device_init(dev.device);
    if(ret != RT_EOK){
        RT_LOGE(TAG,"HC595 Device init err");
        goto hc595test_initerr;
    }

    ret = rt_device_open(dev.device,RT_DEVICE_OFLAG_RDWR);
    if(ret != RT_EOK){
        RT_LOGE(TAG,"HC595 Device open err");
        goto hc595test_initerr;
    }
    uint8_t data = {0x00};
    while(1){
        rt_device_write(dev.device,0,&data,1);
        rt_thread_mdelay(1000);
    }

hc595test_initerr:
    while(1)
    {
        rt_thread_mdelay(10);
    }
}

int hc595test_app(void)
{
    rt_err_t ret;
    ret = rt_thread_init(&dev.hc595_thread,
                            "LoraTH",
                            hc595test_task,
                            NULL,
                            dev.hc595_stack,
                            sizeof(dev.hc595_stack),
                            13, 20);
    if(ret != RT_EOK){
        RT_LOGE(TAG,"74HC595 Thread Init Error!!");
        return RT_ERROR;
    }
    rt_thread_startup(&dev.hc595_thread);
    return RT_EOK;
}
//INIT_APP_EXPORT(hc595test_app);
