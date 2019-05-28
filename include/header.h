//Bibliotecas Externas
#include <ConnectionClass.h>
#include <ArduinoJson.h>
#include <LogClass.h>
#include <EEPROM.h>
#include <HX711.h>

//Bibliotecas Internas
#include "esp32-hal-cpu.h"
#include "soc/rtc.h"
#include "time.h"

//Define configurations do MQTT
const char *SSID = "Positiva";
const char *PASS = "#Positiva2019#";
const char *TOPIC_SUBSCRIBE = "kilos";
const char *TOPIC_PUBLISH = "kilos/value";
const char *BROKER_MQTT = "mqtt.positiva.app";
uint16_t BROKER_PORT = 1883;

//Define configurations do Timer
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = -3600 * 3;

//Define Classes
ConnectionClass Connection;
LogClass Log;
HX711 scale(26, 25);

//Define variáveis
float value = 0;
long last_msg = 0;
long t;

//Define funções
void publish(String _payload1, String _var1, const char *_TOPIC_PUBLISH);
void hx_setup();
void calibrate();
void printLocalTime();

/*
LOGS MQTT ->

#define MQTT_CONNECTION_TIMEOUT     -4
#define MQTT_CONNECTION_LOST        -3
#define MQTT_CONNECT_FAILED         -2
#define MQTT_DISCONNECTED           -1
#define MQTT_CONNECTED               0
#define MQTT_CONNECT_BAD_PROTOCOL    1
#define MQTT_CONNECT_BAD_CLIENT_ID   2
#define MQTT_CONNECT_UNAVAILABLE     3
#define MQTT_CONNECT_BAD_CREDENTIALS 4
#define MQTT_CONNECT_UNAUTHORIZED    5
*/