#include "weather.h"

void get_weather_today()
{
    HTTPClient http;
    String url = "https://restapi.amap.com/v3/weather/weatherInfo?city=510117&key=123456&extensions=base";

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0)
    {
        String payload = http.getString();
        // Serial.println(payload);

        // 解析JSON数据
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        // 获取省份、城市、温度、湿度、天气信息
        String province = doc["lives"][0]["province"];
        String city = doc["lives"][0]["city"];
        String reporttime = doc["lives"][0]["reporttime"];
        float temperature = doc["lives"][0]["temperature"];
        float humidity = doc["lives"][0]["humidity"];
        String weather = doc["lives"][0]["weather"];
        // // 打印信息
        // Serial.print("省份城市: ");
        // Serial.println(province + city);
        // Serial.print("报道时间: ");
        // Serial.println(reporttime);
        // Serial.print("温度: ");
        // Serial.println(temperature);
        // Serial.print("湿度: ");
        // Serial.println(humidity);
        // Serial.print("天气: ");
        // Serial.println(weather);
        // Serial.println();
    }
    else
    {
        Serial.print("Error on HTTP request: ");
        Serial.println(httpCode);
    }
    http.end();
}
void get_weather_forecast()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        String url = "https://restapi.amap.com/v3/weather/weatherInfo?city=510117&key=123456&extensions=all";

        http.begin(url);
        int httpCode = http.GET();

        if (httpCode > 0)
        {
            String payload = http.getString();
            // Serial.println(payload);

            // 解析JSON数据
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);

            // 预报信息
            JsonArray forecasts = doc["forecasts"];
            JsonObject forecast = forecasts[0];
            JsonArray casts = forecast["casts"];
            for (int j = 0; j < 3; ++j)
            { // 使用size()确保我们处理数组中的每个元素
                JsonObject cast = casts[j];

                // 将数据填充到结构体中
                weatherInfos[j].date = cast["date"].as<String>();
                weatherInfos[j].week = cast["week"].as<int>();
                weatherInfos[j].dayWeather = cast["dayweather"].as<String>();
                weatherInfos[j].dayTemp = cast["daytemp"].as<int>();
            }
        }
        http.end();
    }
}