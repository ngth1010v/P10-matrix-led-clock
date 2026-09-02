#include <Arduino.h>
#include "display/P10Driver.h"
#include "display/FontRenderer.h"

#include "storer/Storer.h"
#include "config/ConfigController.h"
#include "timer/Timer.h"

// Module Instances
Storer storer;
ConfigController configController;

P10Driver p10Driver;
Timer timerModule;
FontRenderer fontRenderer;

void setup() {
    Serial.begin(115200);
    delay(500);

    // Initialize storage and config modules
    storer.init();
    configController.init(&storer);

    // Initialize Timer module
    timerModule.init(&storer);

    p10Driver.init();
    fontRenderer.init();

    // Test font rendering
    const char32_t a = 'A';
    FontRenderer::Bitmap testA = fontRenderer.get(a, 16, 8);
    for (int y = 0; y < testA.h; y++) {
        for (int x = 0; x < testA.w; x++) {
            p10Driver.set(x, y, testA.data[y][x]);
            Serial.print(testA.data[y][x]);
            Serial.print(' ');
        }
        Serial.print('\n');
    }
    p10Driver.flush();
}

void loop() {
    delay(1000);

    // Testing and verifying Timer output in main loop
    uint64_t ts = timerModule.getTimestamp();
    Timer::TimeData tData = timerModule.get();

    Serial.printf("[MAIN LOOP] Timestamp: %llu | Date: %04u-%02u-%02u %02u:%02u:%02u (DoW: %u)\n",
                  ts,
                  tData.year, tData.month, tData.day,
                  tData.hour, tData.minute, tData.second,
                  tData.dayOfWeek);
}