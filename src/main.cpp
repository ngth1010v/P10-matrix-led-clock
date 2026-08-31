#include <Arduino.h>
#include "display/P10Driver.h"
#include "wifi/WifiController.h"
#include "timer/Timer.h"

// Module Instances
P10Driver p10Driver;
WifiController wifiController;
Timer timerModule;

void setup() {
    Serial.begin(115200);
    delay(500);

    // Initialize all modules
    p10Driver.init();
    wifiController.init();
    timerModule.init();

    wifiController.addWifi("Huu tri 5G", "0918420922");
    timerModule.setTimezone(7);

    // Initial LED Test Pattern
    p10Driver.set(5, 5, true);
    p10Driver.set(2, 2, true);
    p10Driver.set(50, 15, true);
    p10Driver.flush();
}

void loop() {
    delay(1000);

    Timer::TimeData time = timerModule.get();
    Serial.print(time.hour);
    Serial.print(":");
    Serial.print(time.minute);
    Serial.print(":");
    Serial.print(time.second);
    Serial.print("\n");
}