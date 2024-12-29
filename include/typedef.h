#ifndef __TYPEDEF_H_
#define __TYPEDEF_H_

#include <Arduino.h>

typedef struct // 时钟
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t opt;
} CLOCK_TypeDef;
extern CLOCK_TypeDef clocktime;

typedef struct // 闹钟
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t opt;
} ALARM_TypeDef;
extern ALARM_TypeDef alarmtime;

typedef struct
{
    String date;
    int week;
    String dayWeather;
    int dayTemp;
} Weather_Typedef;
extern Weather_Typedef weatherInfos[10];

// 阵雨 多云 晴 阴 小雨
#endif /* __TYPEDEF_H_ */