#include <M5Stack.h>
#include <utility/M5Timer.h>
#include <avator.h>
#include <SimpleBLE.h>

Avator avator;
M5Timer t;
SimpleBLE ble;
int deg;

void m () {
    Serial.println("called");
    M5.update();
    int p = sin((double)deg * PI / 360) * 100;
    avator.openMouth(p);
    deg = (deg + 20) % 360;
    if (deg == 300)
    {
        avator.openEye(false);
    }
    if (deg == 0)
    {
        avator.openEye(true);
    }
}

void setup() {
    M5.begin();
    ble.begin("Hello World!!");
    avator.init();
    deg = 0;
}

void loop() {
    m();
    delay(100);
}