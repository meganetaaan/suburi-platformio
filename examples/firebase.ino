#include <WiFiClientSecure.h>
#include <FirebaseArduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <M5Stack.h>
#include "const.h"

void connectWifi()
{
  if (WiFi.begin(WIFI_SSID, WIFI_PASSWORD))
  {#include <WiFiClient.h>
    delay(10);
    unsigned long timeout = 10000;
    unsigned long previousMillis = millis();
    while (true)
    {
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis > timeout)
        break;
      if (WiFi.status() == WL_CONNECTED)
        return true;
      delay(1000);
    }
    return false;
  }
}

void setup()
{
    M5.begin();
    int iret = connectWifi();
    if (!iret)
    {
        Serial.println("cannot connect to wifi");
        return;
    }
    Serial.print("connected to: ");
    Serial.println(WiFi.localIP());

    Firebase.begin("publicdata-cryptocurrency.firebase.io.com");
    Firebase.stream("/test/tweet");
}
