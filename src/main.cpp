#include <header.h>

void setup()
{
  Serial.begin(115200);

  Log.begin();
  Log.list_dir("/");
  Log.read_file("/log.txt");

  hx_setup();

  Connection.connect_Wifi(SSID, PASS);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  printLocalTime();

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

    Log.append_file("/log.txt", cstr2);
    Log.append_file("/log.txt", ", ");

    Connection.mqtt_Connect();
    Connection.subscribe_topic();

    i = Connection.mqtt_state();

    str = String(i);
    cstr2 = str.c_str();
    Serial.println(cstr2);

    Log.append_file("/log.txt", cstr2);
    Log.append_file("/log.txt", ", ");

    delay(1000);
  }

  if ((millis() - last_msg > 5000) && (Connection.mqtt_Connected()))
  {
    last_msg = millis();

    scale.power_up();
    Serial.print("\t| average:\t");
    int value = scale.get_units(10);
    Serial.println(value);
    scale.power_down(); 

    publish("value", String(value), TOPIC_PUBLISH);
  }

  Connection.mqtt_Loop();
}
void hx_setup()
{
  scale.begin(19, 18);
  scale.set_scale(292.80);
  scale.tare();
}
void calibrate()
{
  scale.set_scale();
  scale.tare();
  Serial.println("Put known weight on ");
  delay(5000);
  int calibrate_value = scale.get_units(10);
  Serial.println(calibrate_value);
  Serial.print(" Divide this value to the weight and insert it in the scale.set_scale() statement");
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
void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Falha ao obter a hora");
    return;
  }

  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);

  Serial.println(timeStringBuff);

  Log.append_file("/log.txt", "ON: ");
  Log.append_file("/log.txt", timeStringBuff);
  Log.append_file("/log.txt", " ->");
}
void recebe(char *topic, byte *payload, unsigned int length)
{
  if (strcmp(topic, "kilos/calibrate") == 0)
  {
    calibrate();
  }
  else
  {
    scale.tare();
  }
}