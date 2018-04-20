#include <WiFiClient.h>
#include <M5Stack.h>
#include <avator.h>
#include <ArduinoJson.h>
#include <AquesTalkTTS.h>
#include "const.h"
#include <printjp.h>

/*
 ESP8266 Hello World urlencode by Steve Nelson
 URLEncoding is used all the time with internet urls. This is how urls handle funny characters
 in a URL. For example a space is: %20
 These functions simplify the process of encoding and decoding the urlencoded format.
  
 It has been tested on an esp12e (NodeMCU development board)
 This example code is in the public domain, use it however you want. 
  Prerequisite Examples:
  https://github.com/zenmanenergy/ESP8266-Arduino-Examples/tree/master/helloworld_serial
*/
String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}

StaticJsonBuffer<12000> JSONBuffer;
char buffer[256];
char subBuffer[64];
WiFiClient client;

Avator *avator;
int count = 0;
void breath(void *args)
{
  int c = 0;
  for(;;)
  {
    c = c + 1 % 100;
    float f = sin(c * 2 * PI / 100.0);
    avator->setBreath(f);
    delay(33);
  }
}
void drawLoop(void *args)
{
  float last = 0;
  for(;;)
  {
    int level = TTS.getLevel();
    float f = level / 12000.0;
    float open = min(1.0, last + f / 2.0);
    last = f;
    avator->setMouthOpen(open);
    avator->draw();
    delay(33);
  }
}
void saccade(void *args)
{
  for(;;)
  {
    float vertical = (float)rand()/(float)(RAND_MAX / 2) - 1;
    float horizontal = (float)rand()/(float)(RAND_MAX / 2) - 1;
    avator->setGaze(vertical, horizontal);
    delay(500 + 100 * random(20));
  }
}
void blink(void *args)
{
  for(;;)
  {
    avator->setEyeOpen(1);
    delay(2500 + 100 * random(20));
    avator->setEyeOpen(0);
    delay(300 + 10 * random(20));
  }
}

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

String getTweets()
{
  if (!client.connect(HOST, 80)) {
    Serial.println("host connection failed");
    return "[]";
  }
  Serial.println("host connection success");
  String url = "/function-1";
  url += "?word=" + urlencode("技術書典");

  String request = "GET " + url + " HTTP/1.1\r\n"
  + "Host: " + HOST + "\r\n"
  + "Connection: close\r\n\r\n";

  client.print(request);

  int i = 0;
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      String result = client.readStringUntil('\n');
      client.flush();
      return result;
    }
  }
}

void showTweets(String message)
{
  JSONBuffer.clear();
  Serial.println("parse: " + message);
  JsonArray &parsed = JSONBuffer.parseArray(message);
  if (!parsed.success())
  {
    Serial.println("Parsing failed");
    return;
  }
  
  int charNum = M5.Lcd.width() / CHAR_WIDTH;
  M5.Lcd.fillRect(0, 199, M5.Lcd.width(), 40, BLACK);
  for (int i = 0; i < parsed.size(); i++)
  {
    JsonObject &tweet = parsed[i];

    String text = tweet["text"];
    text.toCharArray(buffer, sizeof(buffer));
    for (int j = 0; j < 5; j++)
    {
      char *ptr = &buffer[charNum * j];
      memcpy((void*)subBuffer , ptr, sizeof(subBuffer));
      printJp(0, 199 + CHAR_WIDTH * j, subBuffer);
      memset(subBuffer, 0, sizeof(subBuffer));
    }
    memset(buffer, 0, sizeof(buffer));
    const char *roman = tweet["roman"];
    TTS.play(roman, 100);
    delay(10000);
    M5.Lcd.fillRect(0, 199, M5.Lcd.width(), 40, BLACK);
  }
}

void setupAvator()
{
  avator = new Avator();
  xTaskCreatePinnedToCore(
                    drawLoop,     /* Function to implement the task */
                    "drawLoop",   /* Name of the task */
                    4096,      /* Stack size in words */
                    NULL,      /* Task input parameter */
                    1,         /* Priority of the task */
                    NULL,      /* Task handle. */
                    1);        /* Core where the task should run */
  xTaskCreatePinnedToCore(
                    saccade,     /* Function to implement the task */
                    "saccade",   /* Name of the task */
                    4096,      /* Stack size in words */
                    NULL,      /* Task input parameter */
                    3,         /* Priority of the task */
                    NULL,      /* Task handle. */
                    1);        /* Core where the task should run */
  xTaskCreatePinnedToCore(
                    breath,     /* Function to implement the task */
                    "breath",   /* Name of the task */
                    4096,      /* Stack size in words */
                    NULL,      /* Task input parameter */
                    2,         /* Priority of the task */
                    NULL,      /* Task handle. */
                    1);        /* Core where the task should run */
  xTaskCreatePinnedToCore(
                    blink,     /* Function to implement the task */
                    "blink",   /* Name of the task */
                    4096,      /* Stack size in words */
                    NULL,      /* Task input parameter */
                    2,         /* Priority of the task */
                    NULL,      /* Task handle. */
                    1);        /* Core where the task should run */
}

void setup()
{
  TTS.create(AQUESTALK_KEY);
  M5.begin();
  setupAvator();
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
  M5.update();
  if(M5.BtnA.wasPressed())
  {
    showTweets(getTweets());
  }
  delay(10);
}
