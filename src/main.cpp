#include <header.h>

void setup()
{
  Serial.begin(115200);

  Log.begin();
  Log.list_dir("/");
  Log.read_file("/log.txt");

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

  hx_setup();
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

  if ((millis() - last_msg > 3000) && (Connection.mqtt_Connected()))
  {
    last_msg = millis();

    scale1.power_up();
    scale2.power_up();

    int value1 = scale1.get_units(10);
    int value2 = scale2.get_units(10);

    Serial.print("Balança 1: ");
    Serial.println(value1);

    Serial.print("Balança 2: ");
    Serial.println(value2);
    Serial.println(" ");
    Serial.println(" ");

    scale1.power_down();
    scale2.power_down();

    publish("value1", String(value1), "value2", String(value2), TOPIC_PUBLISH);
  }

  Connection.mqtt_Loop();
}
void hx_setup()
{
  scale1.begin(19, 18);
  scale2.begin(21, 22);

  // calibrate();
  scale1.set_scale(-209.36);
  scale2.set_scale(-209.36);

  scale1.tare();
  scale2.tare();

  // delay(1000);
  // scale1.set_offset(-1070613);
  // scale2.set_offset(-1070613);
  // publish("Offset", String(off), TOPIC_PUBLISH);
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
  publish("calibrate", String(calibrate_value), "", "", TOPIC_PUBLISH);
  delay(10000);
  ESP.restart();
}
void publish(String _payload1, String _var1, String _payload2, String _var2, const char *_TOPIC_PUBLISH)
{
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject &JSONencoder = JSONbuffer.createObject();
  JSONencoder[_payload1] = _var1;
  JSONencoder[_payload2] = _var2;
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
    scale1.tare();
    delay(1000);
    long offset = scale1.get_offset();
    publish("Offset", String(offset), "", "", TOPIC_PUBLISH);
  }
}

signed short Stabilize(signed short value, signed short fdiff, unsigned short fcount)
{
  static unsigned short count_soft = 0xFFFF;
  static unsigned short count_high_difference = 0xFFFF;
  static signed int result = 0;

  signed int dif;

  /* Verify if value of variable is different */
  if (result != value)
  {
    /* Calcule the different */
    dif = value - result;
    /* Verify if difference is higher of fdiff */
    if ((dif > fdiff) || (dif < (fdiff * -1)))
    {
      /* Clear var of lower difference */
      count_soft = 0;
      /* if counter is higher of parameter */
      if ((++count_high_difference) > fcount)
      {
        /* Clear counter */
        count_high_difference = 0;
        /* Make equal sensor and var */
        result = value;
      }
    }
    else
    {
      /* Clear var of lower difference */
      count_high_difference = 0;
      /* if counter is higher of parameter */
      if ((++count_soft) > fcount)
      {
        /* Clear counter */
        count_soft = 0;
        /* Soft rise */
        if (dif < 0)
          --result;
        else
          ++result;
      }
    }
  }
  else
  {
    /* Clear counters */
    count_soft = 0;
    count_high_difference = 0;
    result = value;
  }
  return result;
}
