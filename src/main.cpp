#include <header.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>

#define ANALOG_PIN 36
#define pin13 13
#define pin12 12

Adafruit_ADS1115 ads(0x48);

int RawValue = 0;
float Voltage = 0.0;
float tempC = 0;

void setup()
{
  Serial.begin(115200);

  Connection.wifi_connect(SSID, PASS);

  Connection.set_topic(TOPIC_SUBSCRIBE);
  Connection.func_mac();
  Connection.ota();
  Connection.func_recebe(recebe);

  Connection.set_server(BROKER_PORT, BROKER_MQTT);

  Connection.mqtt_Connect();
  Connection.subscribe_topic();

  ads.begin();
}
void loop()
{
  Connection.handle();

  Connection.wifi_connect(SSID, PASS);

  if (!Connection.mqtt_Connected())
  {

    int i = Connection.mqtt_state();
    String str = String(i);
    const char *cstr2 = str.c_str();
    Serial.println(cstr2);

    Connection.mqtt_Connect();
    Connection.subscribe_topic();

    i = Connection.mqtt_state();

    str = String(i);
    cstr2 = str.c_str();
    Serial.println(cstr2);

    delay(1000);
  }

  if ((millis() - last_msg > 3000) && (Connection.mqtt_Connected()))
  {
    last_msg = millis();

    int16_t adc0 = 0;
    for (size_t i = 0; i < 10; i++)
    {
      adc0 += ads.readADC_SingleEnded(0);
    }
    adc0 /= 10;

    Voltage = (adc0 * 0.1875);
    tempC = Voltage * 0.1;
    Serial.print(adc0);
    Serial.print(" ");
    Serial.print(Voltage);
    Serial.print(" ");
    Serial.println(tempC);
    publish("raw", String(adc0), "tempC", String(tempC), TOPIC_PUBLISH);
  }
  Connection.mqtt_Loop();
}
void hx_setup()
{
  scale1.begin(19, 18);

  scale1.set_gain(32);
}
void publish(String _payload1, String _var1, String _payload2, String _var2, const char *_TOPIC_PUBLISH)
{
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject &JSONencoder = JSONbuffer.createObject();
  JSONencoder[_payload1] = _var1;
  JSONencoder[_payload2] = _var2;
  JSONencoder["MAC"] = "BB:BB:BB:BB:BB:BC";
  JSONencoder["ip"] = Connection.ip();
  char JSONmessageBuffer[100];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Connection.mqtt_Publish(_TOPIC_PUBLISH, JSONmessageBuffer);
  Serial.println(JSONmessageBuffer);
}

void recebe(char *topic, byte *payload, unsigned int length)
{
}
