#include "keydevice.h"
#include "stdio.h"
#include "stdlib.h"

static struct KeyDevice *KeyDevice = NULL;
static uint8_t KeyId = 0;

/* KeyDevice默认使用标准C内存申请 */
static struct SuperRam SetKeyRam = {
	.KeyMalloc = malloc,
	.KeyFree = free,
};

static struct SuperRam *KeyRam = &SetKeyRam;


/* 修改KeyDevice内存操作函数 */
void SetSuperRam(struct SuperRam *SuperRam)
{
	KeyRam = SuperRam;
}

/* 
 * 注册按键函数 
 */
int KeyDeviceRegister(struct KeyDeviceFunction *Key)
{
    struct KeyDevice *p = KeyRam->KeyMalloc(sizeof(struct KeyDevice));
    p->fun = Key;
    p->Cache.Cnt = 0;
    p->Cache.ContinuousCnt = 0;
    p->Cache.ContinuousTimeOutCnt = 0;
    p->Cache.State = Nothing;
    p->Cache.KeyId = KeyId++;
    p->NextKey = NULL;
    
    if(KeyDevice == NULL)
    {
        KeyDevice = p;
    }
    else{
        struct KeyDevice *item = KeyDevice;
        while(item->NextKey != NULL) item = item->NextKey;
        item->NextKey = p;
    }
	return 0;
}

/* 调试部分 */
void PrintKeyList(struct KeyDevice *p)
{
    if(p == NULL)
        return;
    PrintKeyList(p->NextKey);
    printf("\r\nKeyId:%d\r\nKeyDownStatus:%d\r\nShortPressCnt:%d\r\nLongPressCnt:%d\r\n",
            p->Cache.KeyId,p->fun->KeyDown,p->fun->ShortPressCnt,p->fun->LongPressCnt);
}

void PrintKey(void)
{
    PrintKeyList(KeyDevice);
}

/* 释放按键 */
void KeyDeviceListFree(struct KeyDevice *p)
{
	if(p == NULL)
		return;
	KeyDeviceListFree(p->NextKey);
	KeyRam->KeyFree(p);
	p = NULL;
}

/* 删除按键 */
void KeyDevice_Delete(void)
{
	KeyDeviceListFree(KeyDevice);
}

struct KeyDevice *KeyDeviceGetKey(uint8_t KeyId)
{
    struct KeyDevice *item = KeyDevice;
    /* 遍历链表 */
    while(item != NULL){
        if(item->Cache.KeyId == KeyId)
            return item;
        item = item->NextKey;
    }
    return NULL;
}

/* 
 * 根据按键输入状态判断是否有长按短按 
 */
KeyClass KeyDeviceGetStatus(struct KeyDevice *item)
{
    uint8_t KeyValue = item->fun->GetKeyValue();
    if(item == NULL) return NoneKey;
    KeyClass status = NoneKey;
    switch(item->Cache.State)
    {
        case Nothing:
            if(KeyValue == item->fun->KeyDown)
                item->Cache.State = Debounce;
        break;

        case Debounce:
            if(KeyValue == item->fun->KeyDown){
                item->Cache.Cnt++;
                if(item->Cache.Cnt == item->fun->ShortPressCnt)
                    item->Cache.State = Judge;
            }else{
                item->Cache.Cnt = 0;
                item->Cache.State = Nothing;
            }
        break;

        case Judge:
            if(KeyValue == item->fun->KeyDown)
            {
                item->Cache.Cnt++;
                if(item->Cache.Cnt >= item->fun->LongPressCnt){
                    status = LongKey;
                    item->Cache.State = LongPress;
                }
            }else{
                (item->Cache.Cnt >= item->fun->LongPressCnt)?(status = LongKey):(status = ShortKey);
                item->Cache.Cnt = 0;
                item->Cache.State = Nothing;
            }
        break;

        case LongPress:
            if(KeyValue == item->fun->KeyDown){
                status = NoneRelease;
            }else{
                item->Cache.Cnt = 0;
                item->Cache.State = Nothing;
            }
        break;
    }
    return status;
}

void KeyDevice_Tick(void)
{
    KeyClass State;
    struct KeyDevice *item = KeyDevice;
    while(item != NULL){
        State = KeyDeviceGetStatus(item);
        item->Cache.ContinuousTimeOutCnt++;
        switch(State){
            case NoneKey:
                if(item->Cache.ContinuousTimeOutCnt >= item->fun->ContinuousTimeoutCnt){
                        if(item->Cache.ContinuousCnt >= item->fun->ContinuousPressCnt && 
                                    item->fun->ContinuousCallBack != NULL)
                            item->fun->ContinuousCallBack(item->fun->private);
                        else if(item->fun->ShortCallBack != NULL && 
                                    item->Cache.ContinuousCnt > 0)
                            item->fun->ShortCallBack(item->fun->private);
                        item->Cache.ContinuousCnt = 0;
                    }
                if(item->Cache.OldOutState == NoneRelease && 
                    item->fun->KeyLoosedCallBack != NULL)
                {
                    item->fun->KeyLoosedCallBack(item->fun->private);
                }
                item->Cache.OldOutState = NoneKey;
                break;
            case ShortKey:
                item->Cache.ContinuousTimeOutCnt = 0;
                if(item->fun->ContinuousPressCnt > 0){
                    item->Cache.ContinuousCnt++;
                } else {
                    if(item->fun->ShortCallBack != NULL)
                    item->fun->ShortCallBack(item->fun->private);
                }
                break;
            case LongKey:
                if(item->fun->LongCallBack != NULL)
                    item->fun->LongCallBack(item->fun->private);
                break;
            case NoneRelease:
                item->Cache.OldOutState = NoneRelease;
                if(item->fun->NoneReleaseCallBack != NULL)
                    item->fun->NoneReleaseCallBack(item->fun->private);
                break;
        }
        item = item->NextKey;
    }
}