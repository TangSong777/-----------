#include "KEY.h"

typedef struct
{
    int Pin;         // 引脚号
    KEY_STATE State; // 按键状态
    uint8_t *Flag;   // 按键标志位地址
} KEY_TypeDef;

static KEY_TypeDef Keys[KeyN] = {0};

void Key_Init(KEY_INDEX num, int pin, uint8_t *key_flag)
{
    pinMode(pin, INPUT_PULLUP);
    Keys[num].Pin = pin;
    Keys[num].Flag = key_flag;
    Keys[num].State = KEY_CHECK;
}

int Key_read()
{
    for (int i = 0; i < KeyN; i++)
    {
        if (*Keys[i].Flag)
        {
            *Keys[i].Flag = 0;
            return i;
        }
    }
    return -1;
}

void Key_scan(KEY_INDEX num)
{
    switch (Keys[num].State)
    {
    case KEY_CHECK:
    {
        if (!digitalRead(Keys[num].Pin))
            Keys[num].State = KEY_COMFIRM;
        break;
    }
    case KEY_COMFIRM:
    {
        if (!digitalRead(Keys[num].Pin))
        {
            *Keys[num].Flag = 1;
            Keys[num].State = KEY_RELEASE;
        }
        else
            Keys[num].State = KEY_CHECK;
        break;
    }
    case KEY_RELEASE:
    {
        if (digitalRead(Keys[num].Pin))
            Keys[num].State = KEY_CHECK;
        break;
    }
    default:
        break;
    }
}