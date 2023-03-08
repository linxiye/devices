#include "74hc595device.h"
#include "74hc595.h"
#include "drivers/pin.h"


static struct hc595_config *config;

const static char *TAG = "HC595";

void SetDioValue(uint8_t value)
{
    rt_pin_write(config->Diopin,value);
}

void SetRclkValue(uint8_t value)
{
    rt_pin_write(config->Rclpin,value);
}

void SetSclkValue(uint8_t value)
{
    rt_pin_write(config->Sclpin,value);
}

static struct hc595_function fun = {
    .SetDioValue = SetDioValue,
    .SetRclkValue = SetRclkValue,
    .SetSclkValue = SetSclkValue,
};

rt_err_t hc595_device_init(rt_device_t dev)
{
    if(config == NULL){
        RT_LOGE(TAG,"Please Config HC595");
    }
    rt_pin_mode(config->Diopin, PIN_MODE_OUTPUT);
    rt_pin_mode(config->Rclpin, PIN_MODE_OUTPUT);
    rt_pin_mode(config->Sclpin, PIN_MODE_OUTPUT);
    RT_LOGI(TAG,"HC595 Gpio Init Success");

    fun.cascade = config->cascade;
    Hc595Init(&fun);
    RT_LOGI(TAG,"HC595 Device Init Success");
    return RT_EOK;
}

rt_err_t hc595_device_open(rt_device_t dev, rt_uint16_t oflag)
{
    RT_LOGI(TAG,"HC595 Open Success");
    return RT_EOK;
}

rt_err_t hc595_device_close(rt_device_t dev)
{
    RT_LOGI(TAG,"HC595 Close Success");
    return RT_EOK;
}

rt_size_t hc595_device_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    if(size == config->cascade)
        Hc595Set((uint8_t *)buffer);
    else{
        RT_LOGE(TAG,"Please write the data length of the correct concatenation number");
    }
    return RT_EOK;
}

rt_err_t hc595_device_control(rt_device_t dev, int cmd, void *args)
{
    if(cmd == RT_DEVICE_CTRL_CONFIG){
        config = args;
        RT_LOGI(TAG,"HC595 Config Success!!");
    }
    return RT_EOK;
}

static struct rt_device hc595_device = {
    .type = RT_Device_Class_Char,
    .rx_indicate = RT_NULL,
    .tx_complete = RT_NULL,
    .init = hc595_device_init,
    .open = hc595_device_open,
    .close = hc595_device_close,
    .write = hc595_device_write,
    .control = hc595_device_control,
    .user_data = NULL,
};


int hc595_dev_init(void)
{
    rt_err_t ret = RT_EOK;
    ret = rt_device_register(&hc595_device, "hc595", RT_DEVICE_FLAG_RDWR);
    if(ret == RT_ERROR)
        return RT_ERROR;
    return ret;
}
INIT_DEVICE_EXPORT(hc595_dev_init);
