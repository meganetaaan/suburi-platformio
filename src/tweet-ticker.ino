#include <WiFiClientSecure.h>
#include <M5Stack.h>
#include <avator.h>
#include <ArduinoJson.h>
#include "const.h"

StaticJsonBuffer<8000> JSONBuffer;

int connectWifi()
{
  if (WiFi.begin(WIFI_SSID, WIFI_PASSWORD))
  {
    delay(10);
    unsigned long timeout = 10000;
    unsigned long previousMillis = millis();
    while (true)
    {
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis > timeout)
        break;
      if (WiFi.status() == WL_CONNECTED)
        return 0;
      delay(1000);
    }
    return 1;
  }
}

void showTweets()
{
  JSONBuffer.clear();
  JsonArray& parsed = JSONBuffer.parseArray(message);
  if (!parsed.success())
  {
    Serial.println("Parsing failed");
    return;
  }
  Serial.println("Parsing succeed");
  for (int i = 0; i < parsed.size(); i++)
  {
    JsonObject& tweet = parsed[i];
    String roman = tweet["roman"];
    Serial.println(roman);
  }
}

void setup()
{
    M5.begin();
    int iret = connectWifi();
    if (iret != 0)
    {
        Serial.printf("cannot connect to wifi: %d\n", iret);
        return;
    }
    Serial.print("connected to: ");
    Serial.println(WiFi.localIP());
}

void loop()
{
  showTweets();
  delay(1000);
}
