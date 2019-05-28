#include <header.h>

void setup()
{
  Serial.begin(115200);
  delay(1000);
  setCpuFrequencyMhz(80);
  delay(1000);
  Serial.print("Frequencia da CPU: ");
  Serial.println(getCpuFrequencyMhz());
  delay(1000);

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

  Connection.set_server(BROKER_PORT, BROKER_MQTT);
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

  //The magical happens here
  if ((millis() - last_msg > 10000) && (Connection.mqtt_Connected()))
  {
    last_msg = millis();

    value = scale.get_units(10);
    Serial.println(value);
    publish("value", String(value), TOPIC_PUBLISH);
  }

  Connection.mqtt_Loop();

}
void hx_setup()
{
  // Serial.println("");
  // Serial.print("read: \t\t");
  // Serial.println(scale.read()); // print a raw reading from the ADC

  // Serial.print("read average: \t\t");
  // Serial.println(scale.read_average(20)); // print the average of 20 readings from the ADC

  // Serial.print("get value: \t\t");
  // Serial.println(scale.get_value(5)); // print the average of 5 readings from the ADC minus the tare weight (not set yet)

  // Serial.print("get units: \t\t");
  // Serial.println(scale.get_units(5), 1); // print the average of 5 readings from the ADC minus tare weight (not set) divided
  // // by the SCALE parameter (not set yet)

  scale.set_scale(-104.80); // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();             // reset the scale to 0

  // Serial.print("read: \t\t");
  // Serial.println(scale.read()); // print a raw reading from the ADC

  // Serial.print("read average: \t\t");
  // Serial.println(scale.read_average(20)); // print the average of 20 readings from the ADC

  // Serial.print("get value: \t\t");
  // Serial.println(scale.get_value(5)); // print the average of 5 readings from the ADC minus the tare weight, set with tare()

  // Serial.print("get units: \t\t");
  // Serial.println(scale.get_units(5), 1); // print the average of 5 readings from the ADC minus tare weight, divided
  // // by the SCALE parameter set with set_scale
}
void calibrate()
{
  // uncomment if you want to calibrate the bowl
  scale.set_scale();
  scale.tare();
  Serial.println("Put known weight on ");
  delay(5000);
  Serial.print(scale.get_units(10));
  Serial.print(" Divide this value to the weight and insert it in the scale.set_scale() statement");
  while (1 == 1)
    ;
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
