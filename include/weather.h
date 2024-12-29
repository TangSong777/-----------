#ifndef __WEATHER_H_
#define __WEATHER_H_

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "typedef.h"

void get_weather_today();
void get_weather_forecast();

#endif /* __WEATHER_H_ */