#include <header.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>


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
  Connection.set_server(BROKER_PORT, BROKER_MQTT);
  Connection.mqtt_Connect();
  Connection.subscribe_topic();

  ads.begin();
  hx_setup();
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

  if ((millis() - last_msg > 5000) && (Connection.mqtt_Connected()))
  {
    last_msg = millis();

    int16_t adc0 = 0;
    for (size_t i = 0; i < 10; i++)
    {
      adc0 += ads.readADC_SingleEnded(0);
    }
    adc0 /= 10;

    scale1.power_up();

    int strain = scale1.get_units(10);

    Voltage = (adc0 * 0.1875);
    tempC = Voltage * 0.1;

    Serial.print("Balança 1: ");
    Serial.print(strain);
    Serial.print(" ");
    Serial.print(adc0);
    Serial.print(" ");
    Serial.print(Voltage);
    Serial.print(" ");
    Serial.println(tempC);

    publish("tempRaw", String(adc0), "tempC", String(tempC), "strain", String(strain), TOPIC_PUBLISH);
  }
  Connection.mqtt_Loop();
}
void hx_setup()
{
  scale1.begin(19, 18);

  scale1.set_gain(32
  );
}
void publish(String _payload1, String _var1, String _payload2, String _var2, String _payload3, String _var3, const char *_TOPIC_PUBLISH)
{
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject &JSONencoder = JSONbuffer.createObject();
  JSONencoder[_payload1] = _var1;
  JSONencoder[_payload2] = _var2;
  JSONencoder[_payload3] = _var3;
  JSONencoder["mac"] = Connection.func_mac();
  JSONencoder["ip"] = Connection.ip();
  char JSONmessageBuffer[100];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Connection.mqtt_Publish(_TOPIC_PUBLISH, JSONmessageBuffer);
  Serial.println(JSONmessageBuffer);
}