# devices 外设组件


## 目录结构
```
├── 4G                                  /* 4G网络类设备 */
|   └── GobiNet                         /* 移远GobiNetUSB驱动 */
├── energy                              /* 电源检测芯片类外设 */
|   ├── cw2015chbd                      /* CW2015CHBD驱动 */
|   └── max17043                        /* 美信电池管理芯片MAX17043驱动 */
├── gyroscope                           /* 陀螺仪类外设 */
|   └── adxl                            /* adxl345三轴陀螺仪驱动 */
├── ioexpand                            /* IO扩展类外设 */
|   └── 74hc595                         /* 按键驱动函数 */
├── keydevice                           /* 通用按键驱动库 */
├── Lora                                /* Lora类外设 */
|   └── llcc68                          /* LLCC68驱动库 */
├── rfid                                /* rfid类外设 */
|   └── fm17550                         /* fm17550驱动库（I2C方式） */
├── sensor                              /* 传感器类外设 */
|   └── cs1237                          /* CS1237差分电压采集驱动库 */
├── storage                             /* 容量类外设 */
|   └── norflash                        /* lvgl 与 lcd驱动对接函数 */
├── wifi                                /* WIFI网络类设备 */
|   ├── rtl8188eu                       /* rtl8188eu USB驱动 */
|   ├── rtl8189fs                       /* rtl8189fs SDIO驱动 */ 
|   └── rtl8723bs                       /* rtl8723bs SDIO驱动 */
└── README.md
```
