#include <header.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>

#define ANALOG_PIN 36

Adafruit_ADS1115 ads(0x48);

int RawValue = 0;
float Voltage = 0.0;
float tempC = 0;

void setup()
{
  Serial.begin(115200);

  Connection.connect_Wifi(SSID, PASS);

  Connection.set_topic(TOPIC_SUBSCRIBE);
  Connection.func_mac();
  Connection.ota();
  Connection.func_recebe(recebe);

  Connection.set_server(BROKER_PORT, BROKER_MQTT);

  Connection.mqtt_Connect();
  Connection.subscribe_topic();

  Connection.set_topic(TOPIC_SUBSCRIBE_CALIBRATE);
  Connection.subscribe_topic();

  Connection.set_topic(TOPIC_SUBSCRIBE_TARE);
  Connection.subscribe_topic();

  hx_setup();

  ads.begin();
}
void loop()
{
  Connection.handle();

  Connection.connect_Wifi(SSID, PASS);

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
    adc0 = ads.readADC_SingleEnded(0);
    Voltage = (adc0 * 0.1875);
    tempC = Voltage * 0.1;
    Serial.print(adc0);
    Serial.print(" ");
    Serial.print(Voltage);
    Serial.print(" ");
    Serial.println(tempC);

    scale1.power_up();
    int value1 = scale1.get_units(10);

    Serial.print("Balança 1: ");
    Serial.println(value1);

    publish("value1", String(value1), TOPIC_PUBLISH);
  }

  Connection.mqtt_Loop();
  delay(500);
}
void hx_setup()
{
  scale1.begin(19, 18);

  scale1.set_gain(32);

}
void calibrate()
{
  scale1.set_scale();
  scale1.tare();
  Serial.println("Put known weight on ");
  delay(5000);
  int calibrate_value = scale1.get_units(10);
  Serial.println(calibrate_value);
  Serial.print(" Divide this value to the weight and insert it in the scale1.set_scale1() statement");
  delay(10000);
  publish("calibrate", String(calibrate_value), TOPIC_PUBLISH);
  delay(10000);
  ESP.restart();
}
void publish(String _payload1, String _var1, const char *_TOPIC_PUBLISH)
{
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject &JSONencoder = JSONbuffer.createObject();
  JSONencoder[_payload1] = _var1;
  JSONencoder["ip"] = Connection.ip();
  char JSONmessageBuffer[100];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Connection.mqtt_Publish(_TOPIC_PUBLISH, JSONmessageBuffer);
  Serial.println(JSONmessageBuffer);
}

void recebe(char *topic, byte *payload, unsigned int length)
{
  if (strcmp(topic, "kilos/calibrate") == 0)
  {
    calibrate();
  }
  else
  {
    scale1.tare();
    delay(1000);
    long offset = scale1.get_offset();
    publish("Offset", String(offset), TOPIC_PUBLISH);
  }
}
