#ifndef __KEYDEVICE_H__
#define __KEYDEVICE_H__

#include "stdint.h"
#include "stdlib.h"

#define KEY_OK              0       /* 返回成功 */
#define KEY_MALLOC_ERROR    1       /* 内存申请失败 */

struct SuperRam {
      void *(*KeyMalloc)(size_t sz);
      void (*KeyFree)(void *ptr);
};


typedef enum KeyClass_t{
    NoneKey = 0,            /* 无状态 */
    ShortKey = 1,           /* 短按 */
    LongKey = 2,            /* 长按 */
    NoneRelease = 3,        /* 长按识别之后继续长按 */
}KeyClass;


typedef enum KeyState_t{
    Nothing = 0,            /* 无状态(初始化状态) */
    Debounce = 1,           /* 消抖状态 */
    Judge = 2,              /* 判断是否长按 */
    LongPress = 3,          /* 判断为长按后还一直按着 */
}KeyState;

typedef enum KeyDownStatus_t{
    KeyDownIsLow = 0,       /* 低电平有效 */
    KeyDownIsHight = 1,     /* 高电平有效 */
}KeyDownStatus;

/* 
 * 按键设备结构体
 * 此结构体不包含连按判断
 * 只有长短按判断
 */
struct KeyDeviceFunction {
    KeyDownStatus KeyDown;              /* 
                                         * 按键按下状态
                                         * 0：表示按键按下的时候为0
                                         * 1：表示按键按下的时候为1
                                        */

    uint8_t ShortPressCnt;              /* 短按所需次数(实际就是短按消抖) */
    uint16_t LongPressCnt;               /* 长按所需次数 */

    uint8_t ContinuousPressCnt;         /* 连按次数 */
    uint8_t ContinuousTimeoutCnt;       /* 连按超时判断(设定短按会用这个延时后才会上报) */

    uint8_t (*GetKeyValue)(void);                   /* 按键值获取函数 */
    void (*ShortCallBack)(void *private);           /* 短按回调函数 */
    void (*LongCallBack)(void *private);            /* 长按回调函数 */
    void (*NoneReleaseCallBack)(void *private);     /* 长按不松开 */
    void (*ContinuousCallBack)(void *private);      /* 连按回调函数 */
    void (*KeyLoosedCallBack)(void *private);       /* 长按按键松开回调函数 */
    void *private;                                  /* 用户私有数据 */
};

struct KeyDeviceCache {
    uint8_t KeyId;                      /* 按键ID */
    uint16_t Cnt;                       /* 按键计数 */
    uint8_t ContinuousCnt;              /* 连按计数 */
    uint8_t ContinuousTimeOutCnt;       /* 连按超时计算 */
    KeyState State;                     /* 按键状态 */
    KeyClass OldOutState;               /* 按键上次输出状态 */
};

struct KeyDevice {
    struct KeyDeviceFunction *fun;      /* 按键设备 */
    struct KeyDeviceCache    Cache;     /* 按键缓存数据 */
    struct KeyDevice        *NextKey;   /* 链表指向 */
};

/*
 * 修改按键内存操作函数
 */
void SetSuperRam(struct SuperRam *SuperRam);

/* 
 * 注册按键函数
 */
int KeyDeviceRegister(struct KeyDeviceFunction *Key);

void PrintKey(void);

/* 删除整组按键 */
void KeyDevice_Delete(void);

/* 按键心跳 */
void KeyDevice_Tick(void);
#endif
