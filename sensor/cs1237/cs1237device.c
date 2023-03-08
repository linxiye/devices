#include "cs1237device.h"
#include "drivers/pin.h"
#include "useridle.h"
#include "string.h"

const static char *TAG = "CS1237";
static struct cs1237_dev_config *config;


void cs1237_device_ms(uint32_t ms)
{
    rt_thread_mdelay(ms);
}

uint8_t cs1237_get_sda(void)
{
    return rt_pin_read(config->Sdapin);
}

void cs1237_set_scl(uint8_t value)
{
    rt_pin_write(config->Sclpin, value);
}

void cs1237_set_sda(uint8_t value)
{
    rt_pin_write(config->Sdapin, value);
}

void cs1237_sda_input(void)
{
    rt_pin_mode(config->Sdapin, PIN_MODE_INPUT);
}

void cs1237_sda_output(void)
{
    rt_pin_mode(config->Sdapin, PIN_MODE_OUTPUT);
}

void sda_irq_disable(void)
{
    rt_pin_irq_enable(config->Sdapin,DISABLE);
}

void sda_irq_enable(void)
{
    rt_pin_irq_enable(config->Sdapin,ENABLE);
}

void cs1237_delay_us(uint32_t value)
{
    rt_hw_us_delay(value);
}

static struct cs1237_fun fun = {
    .delay_ms = cs1237_device_ms,
    .delay_us = cs1237_delay_us,
    .get_sda_data = cs1237_get_sda,
    .scl_output_data = cs1237_set_scl,
    .sda_output_data = cs1237_set_sda,
    .set_sda_input = cs1237_sda_input,
    .set_sda_output = cs1237_sda_output,
    .sda_irq_disable = sda_irq_disable,
    .sda_irq_enable = sda_irq_enable,
};

static struct ideldeal ideldeal = {
    .Next = NULL,
    .Parent = NULL,
    .status = fun_stop,
    .function = cs1237_sda_irq_callback,
};

void cs1237_sda_irq(void *argv)
{
    if(ideldeal.status == fun_stop)
        ideldeal.status = fun_run;
}

rt_err_t cs1237_device_init(rt_device_t dev)
{
    rt_err_t ret;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource0);

    rt_pin_mode(config->Sclpin, PIN_MODE_OUTPUT);
    rt_pin_mode(config->Sdapin, PIN_MODE_OUTPUT);

    RT_LOGI(TAG,"CS1237 Gpio Init Success");

    if(cs1237_init(&fun,config->config) != 0){
        RT_LOGE(TAG,"CS1237 Device Init Error");
        return RT_ERROR;
    }

    useridle_register(&ideldeal);
    RT_LOGI(TAG,"CS1237 Register ideldeal Success");


    ret = rt_pin_attach_irq(config->Sdapin, PIN_IRQ_MODE_FALLING,
                cs1237_sda_irq, NULL);
    if(ret != RT_EOK){
        RT_LOGE(TAG,"cs1237 irq init fail");
        return RT_ERROR;
    }
    RT_LOGI(TAG,"CS1237 Set Sda Irq Falling Success");
    cs1237_start();
    RT_LOGI(TAG,"CS1237 Device Init Success");

    return RT_EOK;
}

rt_err_t cs1237_device_open(rt_device_t dev, rt_uint16_t oflag)
{
    RT_LOGI(TAG,"CS1237 Open Success");
    return RT_EOK;
}

rt_err_t cs1237_device_close(rt_device_t dev)
{
    cs1237_stop();
    RT_LOGI(TAG,"CS1237 Close Success");
    return RT_EOK;
}

rt_size_t cs1237_device_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    int value = cs1237_read_value();
    if(size != 4)
        return 0;
    if(value & 0x00800000)
        value = -(~value & 0x007FFFFF);
    rt_memcpy(buffer, &value, size);
    return size;
}

rt_err_t cs1237_device_control(rt_device_t dev, int cmd, void *args)
{
    if(cmd == RT_DEVICE_CTRL_CONFIG){
        config = args;
        RT_LOGI(TAG,"CS1237 Config Success!!");
    }
    return RT_EOK;
}

static struct rt_device cs1237_device = {
    .type = RT_Device_Class_Char,
    .rx_indicate = RT_NULL,
    .tx_complete = RT_NULL,
    .init = cs1237_device_init,
    .open = cs1237_device_open,
    .close = cs1237_device_close,
    .read = cs1237_device_read,
    .control = cs1237_device_control,
    .user_data = NULL,
};

int cs1237_dev_init(void)
{
    rt_err_t ret = RT_EOK;
    ret = rt_device_register(&cs1237_device, "cs1237", RT_DEVICE_FLAG_RDWR);
    if(ret == RT_ERROR)
        return RT_ERROR;
    return ret;
}
//INIT_DEVICE_EXPORT(cs1237_dev_init);
