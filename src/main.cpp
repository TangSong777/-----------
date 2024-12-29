#include <Arduino.h>
#include <Ticker.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoMqttClient.h>
#include <Temperature_LM75_Derived.h>
#include <time.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "KEY.h"
#include "LCD.h"
#include "typedef.h"
#include "weather.h"

#define Board_LED_Pin 48 // 板载LED，低电平触发
#define Board_KEY_Pin 0  // 板载KEY，低电平触发

#define ADC_Pin 1
#define BEEP_Pin 2

#define LED0_Pin 4
#define LED1_Pin 5
#define LED2_Pin 6
#define LED3_Pin 7

#define KEY0_Pin 8
#define KEY1_Pin 9
#define KEY2_Pin 10
#define KEY3_Pin 11

#define LM75_SCL 17
#define LM75_SDA 18

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "cn.pool.ntp.org", 8 * 3600);

Ticker ticker1;
Ticker ticker2;
Ticker ticker3;
Ticker ticker4;
Ticker ticker5;

Generic_LM75 temperature;

CLOCK_TypeDef clocktime = {0, 0, 0, 0};
ALARM_TypeDef alarmtime = {11, 45, 14, 0};
Weather_Typedef weatherInfos[10];

uint8_t Key_flag[5] = {0};
bool ifSendData = false;
bool ifRing = false;
uint8_t ring_index = 0;
uint8_t progressBar_num = 0;

float Temp = 0.0;

const char *wifi_ssid = "YHL_Wifi";
const char *wifi_password = "123456";

String week = "";      // 定义字符串变量 week，存放星期信息
time_t t;              // 定义 time_t 类型变量，存放时间戳
struct tm *current_tm; // 定义 tm 类型变量，存放分解后的时间信息

const char broker[] = "iot-123456.mqtt.iothub.aliyuncs.com";
int port = 1883;
const char outTopic[] = "/sys/123456/ESP32_dev/thing/event/property/post";
const long interval = 1000;
unsigned long previousMillis = 0;
String inputString = "";

uint8_t func_index = 0;

void (*current_operation_index)();
void key_init();
void led_init();
void KeyTask();
void ProgressBarTask();
void TempGetTask();
void TimeTask();
void RingTask();
void clocktimeUpdate();
void mqtt_init();
void mqtt_sendData();
void mqtt_getData(int messageSize);
void LCD_Drawclock(uint8_t x, uint8_t y, CLOCK_TypeDef *clock, uint16_t color, uint16_t bgColor, uint16_t optColor, const FontDef *font);
void LCD_Drawalarm(uint8_t x, uint8_t y, ALARM_TypeDef *alarm, uint16_t color, uint16_t bgColor, uint16_t optColor, const FontDef *font);
void LCD_DrawWeather(uint8_t x, uint8_t y, Weather_Typedef *weatherInfos, uint16_t color, uint16_t bgColor, const FontDef *font);
void smartConfig();

void setup()
{
    Serial.begin(115200);
    Wire.begin(LM75_SDA, LM75_SCL);
    led_init();
    key_init();
    pinMode(BEEP_Pin, OUTPUT); // 初始化BEEP引脚
    digitalWrite(BEEP_Pin, LOW);

    WiFi.begin(wifi_ssid, wifi_password);
    // smartConfig();
    delay(2000);

    ticker1.attach_ms(10, KeyTask);
    ticker2.attach(2, TempGetTask);
    ticker5.attach_ms(100, RingTask);

    if (WiFi.status() == WL_CONNECTED)
        clocktimeUpdate();
    ticker4.attach(1, TimeTask);

    if (WiFi.status() == WL_CONNECTED)
    {
        mqtt_init();
        get_weather_forecast();
    }
    LCD_Init();
    ticker3.attach_ms(10, ProgressBarTask);
    while (ticker3.active())
    {
        LCD_DrawNumbers(57, 60, (int)100 * LCD_DrawProgressBar(13, 75, 102, 10, LCD_RED, LCD_BLACK, LCD_WHITE, progressBar_num, 100), 3, LCD_RED, LCD_BLACK, &Font_6x12);
    }
    LCD_FillScreen(LCD_BLACK);
}
void loop()
{
    if (clocktime.hour == alarmtime.hour && clocktime.minute == alarmtime.minute && clocktime.second == alarmtime.second)
    {
        ifRing = true;
        ring_index = 0;
    }
    if (WiFi.status() == WL_CONNECTED)
        mqttClient.poll(); // 保持连接

    if (ifSendData)
        mqtt_sendData();

    switch (Key_read())
    {
    case KEY0:
        (++func_index) %= 3;
        clocktime.opt = alarmtime.opt = 0;
        break;
    case KEY1:
        switch (func_index)
        {
        case 1:
            ++clocktime.opt %= 3;
            break;
        case 2:
            ++alarmtime.opt %= 3;
            break;
        default:
            break;
        }
        break;
    case KEY2:
        switch (func_index)
        {
        case 1:
            switch (clocktime.opt)
            {
            case 0:
                ++clocktime.hour %= 24;
                break;
            case 1:
                ++clocktime.minute %= 60;
                break;
            case 2:
                clocktime.second = 0;
                break;
            default:
                break;
            }
            break;
        case 2:
            switch (alarmtime.opt)
            {
            case 0:
                ++alarmtime.hour %= 24;
                break;
            case 1:
                ++alarmtime.minute %= 60;
                break;
            case 2:
                ++alarmtime.second %= 60;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
        break;
    case KEY3:
        ifRing = 0;
        noTone(BEEP_Pin);
        break;
    case Board_KEY:
        if (ifSendData)
            ifSendData = false;
        else
            ifSendData = true;
        break;
    default:
        break;
    }
    // current_operation_index = table[func_index].current_operation; // 执行当前索引号所对应的功能函数
    // (*current_operation_index)();

    LCD_Drawclock(32, 0, &clocktime, LCD_GREEN, LCD_BLACK, LCD_RED, &Font_8x16);
    LCD_Drawalarm(32, 32, &alarmtime, LCD_GREEN, LCD_BLACK, LCD_RED, &Font_8x16);

    LCD_DrawWeather(66, 94, &weatherInfos[0], LCD_YELLOW, LCD_RED, &Font_6x12);
    LCD_DrawWeather(66, 110, &weatherInfos[1], LCD_YELLOW, LCD_RED, &Font_6x12);
    LCD_DrawWeather(66, 126, &weatherInfos[2], LCD_YELLOW, LCD_RED, &Font_6x12);

    LCD_DrawString(0, 94, weatherInfos[0].date.c_str(), LCD_YELLOW, LCD_RED, &Font_6x12);
    LCD_DrawString(0, 110, weatherInfos[1].date.c_str(), LCD_YELLOW, LCD_RED, &Font_6x12);
    LCD_DrawString(0, 126, weatherInfos[2].date.c_str(), LCD_YELLOW, LCD_RED, &Font_6x12);
    LCD_DrawString(0, 140, "Temp:", LCD_YELLOW, LCD_RED, &Font_8x16);
    LCD_DrawNumbers(40, 140, (int)Temp, 2, LCD_YELLOW, LCD_RED, &Font_8x16);
}
void key_init()
{
    Key_Init(KEY0, KEY0_Pin, &Key_flag[0]);
    Key_Init(KEY1, KEY1_Pin, &Key_flag[1]);
    Key_Init(KEY2, KEY2_Pin, &Key_flag[2]);
    Key_Init(KEY3, KEY3_Pin, &Key_flag[3]);
    Key_Init(Board_KEY, Board_KEY_Pin, &Key_flag[4]);
}
void led_init()
{
    pinMode(LED0_Pin, OUTPUT);
    pinMode(LED1_Pin, OUTPUT);
    pinMode(LED2_Pin, OUTPUT);
    pinMode(LED3_Pin, OUTPUT);
    pinMode(Board_LED_Pin, OUTPUT);
}
void KeyTask()
{
    Key_scan(KEY0);
    Key_scan(KEY1);
    Key_scan(KEY2);
    Key_scan(KEY3);
    Key_scan(Board_KEY);
}
void TempGetTask()
{
    Temp = temperature.readTemperatureC();
}
void ProgressBarTask()
{
    if (progressBar_num < 100)
        progressBar_num += 1;
    else
        ticker3.detach();
}
void TimeTask()
{
    clocktime.second++;
    if (clocktime.second == 60)
    {
        clocktime.second = 0;
        clocktime.minute++;
        if (clocktime.minute == 60)
        {
            clocktime.minute = 0;
            clocktime.hour++;
            if (clocktime.hour == 24)
                clocktime.hour = 0;
        }
    }
}
void RingTask()
{
    if (ifRing)
    {
        ++ring_index %= 20;
        tone(BEEP_Pin, ring_index * 100);
    }
}
void clocktimeUpdate()
{
    timeClient.begin();            // 启动时间服务器
    timeClient.update();           // 更新时间
    t = timeClient.getEpochTime(); // 获得时间戳
    current_tm = localtime(&t);    // 将时间戳转换为当前时间，带时区
    clocktime.hour = current_tm->tm_hour;
    clocktime.minute = current_tm->tm_min;
    clocktime.second = current_tm->tm_sec; // 将时间赋值到自定义结构体变量中
}
void mqtt_init()
{
    mqttClient.setId("123456.ESP32_dev|securemode=2,signmethod=123456,timestamp=1734441138811|");
    mqttClient.setUsernamePassword("ESP32_dev&123456", "123456");

    if (!mqttClient.connect(broker, port))
    {
        Serial.print("MQTT connection failed! Error code = ");
        Serial.println(mqttClient.connectError());
        while (1)
            ;
    }
    mqttClient.onMessage(mqtt_getData);
}
void mqtt_sendData()
{
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval)
    {
        // save the last time a message was sent
        previousMillis = currentMillis;

        String payload;

        //{"params":{"temp":1.22,"humi":22},"version":"1.0.0"}

        DynamicJsonDocument json_msg(512);
        DynamicJsonDocument json_data(512);

        json_data["Temp"] = (float)Temp;
        json_data["Humi"] = (float)0;
        json_data["LED"] = (int)!digitalRead(Board_LED_Pin);

        json_msg["params"] = json_data;
        json_msg["version"] = "1.0.0";

        serializeJson(json_msg, payload);

        Serial.print("Sending message to topic: ");
        Serial.println(outTopic);
        Serial.println(payload);

        bool retained = false;
        int qos = 1;
        bool dup = false;

        mqttClient.beginMessage(outTopic, payload.length(), retained, qos, dup);
        mqttClient.print(payload);
        mqttClient.endMessage();

        Serial.println();
    }
}
void mqtt_getData(int messageSize)
{
    // we received a message, print out the topic and contents
    // Serial.print("Received a message with topic '");
    // Serial.print(mqttClient.messageTopic());
    // Serial.print("', duplicate = ");
    // Serial.print(mqttClient.messageDup() ? "true" : "false");
    // Serial.print(", QoS = ");
    // Serial.print(mqttClient.messageQoS());
    // Serial.print(", retained = ");
    // Serial.print(mqttClient.messageRetain() ? "true" : "false");
    // Serial.print("', length ");
    // Serial.print(messageSize);
    // Serial.println(" bytes:");

    while (mqttClient.available())
    {
        char inChar = (char)mqttClient.read();
        inputString += inChar;
        if (inputString.length() == messageSize)
        {

            DynamicJsonDocument json_msg(1024);
            DynamicJsonDocument json_item(1024);
            DynamicJsonDocument json_ifget(100);
            DynamicJsonDocument json_led(100);

            deserializeJson(json_msg, inputString);
            String ifGet = json_msg["Get"];
            String led = json_msg["LED"];

            deserializeJson(json_ifget, ifGet);
            deserializeJson(json_led, led);

            bool value_ifget = json_ifget["value"];
            bool value_led = json_led["value"];

            if (value_ifget)
            {
                if (value_led == 0)
                {
                    // 关
                    Serial.println("off");
                    digitalWrite(Board_LED_Pin, HIGH);
                }
                else
                {
                    // 开
                    Serial.println("on");
                    digitalWrite(Board_LED_Pin, LOW);
                }
            }
            inputString = "";
        }
    }
}
void LCD_Drawclock(uint8_t x, uint8_t y, CLOCK_TypeDef *clock, uint16_t color, uint16_t bgColor, uint16_t optColor, const FontDef *font)
{
    if (func_index == 1)
    {
        switch (clock->opt)
        {
        case 0:
            LCD_DrawPaddedNumbers(x, y, clock->hour, 2, optColor, bgColor, font);
            LCD_DrawString(x + 2 * font->width, y, ":", color, bgColor, font);
            LCD_DrawPaddedNumbers(x + 3 * font->width, y, clock->minute, 2, color, bgColor, font);
            LCD_DrawString(x + 5 * font->width, y, ":", color, bgColor, font);
            LCD_DrawPaddedNumbers(x + 6 * font->width, y, clock->second, 2, color, bgColor, font);
            break;
        case 1:
            LCD_DrawPaddedNumbers(x, y, clock->hour, 2, color, bgColor, font);
            LCD_DrawString(x + 2 * font->width, y, ":", color, bgColor, font);
            LCD_DrawPaddedNumbers(x + 3 * font->width, y, clock->minute, 2, optColor, bgColor, font);
            LCD_DrawString(x + 5 * font->width, y, ":", color, bgColor, font);
            LCD_DrawPaddedNumbers(x + 6 * font->width, y, clock->second, 2, color, bgColor, font);
            break;
        case 2:
            LCD_DrawPaddedNumbers(x, y, clock->hour, 2, color, bgColor, font);
            LCD_DrawString(x + 2 * font->width, y, ":", color, bgColor, font);
            LCD_DrawPaddedNumbers(x + 3 * font->width, y, clock->minute, 2, color, bgColor, font);
            LCD_DrawString(x + 5 * font->width, y, ":", color, bgColor, font);
            LCD_DrawPaddedNumbers(x + 6 * font->width, y, clock->second, 2, optColor, bgColor, font);
            break;
        default:
            break;
        }
    }
    else
    {
        LCD_DrawPaddedNumbers(x, y, clock->hour, 2, color, bgColor, font);
        LCD_DrawString(x + 2 * font->width, y, ":", color, bgColor, font);
        LCD_DrawPaddedNumbers(x + 3 * font->width, y, clock->minute, 2, color, bgColor, font);
        LCD_DrawString(x + 5 * font->width, y, ":", color, bgColor, font);
        LCD_DrawPaddedNumbers(x + 6 * font->width, y, clock->second, 2, color, bgColor, font);
    }
}
void LCD_Drawalarm(uint8_t x, uint8_t y, ALARM_TypeDef *alarm, uint16_t color, uint16_t bgColor, uint16_t optColor, const FontDef *font)
{
    if (func_index == 2)
    {
        switch (alarm->opt)
        {
        case 0:
            LCD_DrawPaddedNumbers(x, y, alarm->hour, 2, optColor, bgColor, font);
            LCD_DrawString(x + 2 * font->width, y, ":", color, bgColor, font);
            LCD_DrawPaddedNumbers(x + 3 * font->width, y, alarm->minute, 2, color, bgColor, font);
            LCD_DrawString(x + 5 * font->width, y, ":", color, bgColor, font);
            LCD_DrawPaddedNumbers(x + 6 * font->width, y, alarm->second, 2, color, bgColor, font);
            break;
        case 1:
            LCD_DrawPaddedNumbers(x, y, alarm->hour, 2, color, bgColor, font);
            LCD_DrawString(x + 2 * font->width, y, ":", color, bgColor, font);
            LCD_DrawPaddedNumbers(x + 3 * font->width, y, alarm->minute, 2, optColor, bgColor, font);
            LCD_DrawString(x + 5 * font->width, y, ":", color, bgColor, font);
            LCD_DrawPaddedNumbers(x + 6 * font->width, y, alarm->second, 2, color, bgColor, font);
            break;
        case 2:
            LCD_DrawPaddedNumbers(x, y, alarm->hour, 2, color, bgColor, font);
            LCD_DrawString(x + 2 * font->width, y, ":", color, bgColor, font);
            LCD_DrawPaddedNumbers(x + 3 * font->width, y, alarm->minute, 2, color, bgColor, font);
            LCD_DrawString(x + 5 * font->width, y, ":", color, bgColor, font);
            LCD_DrawPaddedNumbers(x + 6 * font->width, y, alarm->second, 2, optColor, bgColor, font);
            break;
        default:
            break;
        }
    }
    else
    {
        LCD_DrawPaddedNumbers(x, y, alarm->hour, 2, color, bgColor, font);
        LCD_DrawString(x + 2 * font->width, y, ":", color, bgColor, font);
        LCD_DrawPaddedNumbers(x + 3 * font->width, y, alarm->minute, 2, color, bgColor, font);
        LCD_DrawString(x + 5 * font->width, y, ":", color, bgColor, font);
        LCD_DrawPaddedNumbers(x + 6 * font->width, y, alarm->second, 2, color, bgColor, font);
    }
}
void LCD_DrawWeather(uint8_t x, uint8_t y, Weather_Typedef *weatherInfo, uint16_t color, uint16_t bgColor, const FontDef *font)
{
    if (weatherInfo->dayWeather.equals("晴"))
        LCD_DrawString(x, y, "sunny", color, bgColor, font);
    else if (weatherInfo->dayWeather.equals("阵雨") || weatherInfo->dayWeather.equals("小雨"))
        LCD_DrawString(x, y, "rainy", color, bgColor, font);
    else if (weatherInfo->dayWeather.equals("阴") || weatherInfo->dayWeather.equals("多云"))
        LCD_DrawString(x, y, "cloudy", color, bgColor, font);
}
void smartConfig()
{
    WiFi.mode(WIFI_STA); // 设置WIFI模块为STA模式
    Serial.println("\r\nWaiting for connection");
    // smartconfig进行初始化
    WiFi.beginSmartConfig();
    while (1) // 等待连接成功 ，如果未连接成功LED就每隔500ms闪烁
    {
        Serial.print(">");
        digitalWrite(LED0_Pin, !digitalRead(LED0_Pin));
        delay(500);
        // 如果连接成功后就打印出连接的WIFI信息
        if (WiFi.smartConfigDone())
        {
            Serial.println("\r\nSmartConfig Success");
            Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
            Serial.printf("PW:%s\r\n", WiFi.psk().c_str()); // 打印出密码
            break;
        }
    }
}
