#include "soc/rtc.h"
#include <ConnectionClass.h>
#include <ArduinoJson.h>
#include "esp32-hal-cpu.h"
#include "HX711.h"

#include <LogClass.h>

#include <EEPROM.h>
#include "time.h"


//Define configurations
const char *SSID = "Positiva";
const char *PASS = "#Positiva2019#";
const char *TOPIC_SUBSCRIBE = "kilos";
const char *TOPIC_PUBLISH = "kilos/value";
const char *BROKER_MQTT = "mqtt.positiva.app";
uint16_t BROKER_PORT = 1883;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;

//nao esqueca de ajustar o fuso
const int daylightOffset_sec = -3600 * 3;

ConnectionClass Connection;
LogClass Log;

float value = 0;

long last_msg = 0;

void publish(String _payload1, String _var1, const char *_TOPIC_PUBLISH);
void hx_setup();

long t;

void calibrate();
void printLocalTime();
void publish(String _payload1, String _var1, String _payload2, int _var2, const char *_TOPIC_PUBLISH);
HX711 scale(26, 25);