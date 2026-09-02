#include <Arduino.h>
#include "display/P10Driver.h"
#include "display/FontRenderer.h"

#include "storer/Storer.h"
#include "config/ConfigController.h"
#include "timer/Timer.h"
#include "display/Animator.h"

// Module Instances
Storer storer;
ConfigController configController;

P10Driver p10Driver;
Timer timerModule;
FontRenderer fontRenderer;
Animator animator;

// Animation test state counters
static int counter = 0;

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

    // Initialize Animator module
    animator.init(&p10Driver, &fontRenderer);

    // Test initial static rendering
    FontRenderer::Bitmap testA = fontRenderer.get('A', true);
    for (int x = 0; x < testA.w; x++) {
        for (int y = 0; y < testA.h; y++) {
            p10Driver.set(x + 4, y, testA.pixels[x][y]);
        }
    }
    p10Driver.flush();
    
    Serial.println("[SETUP] Setup completed. Starting animation sequence in loop...");
}

void loop() {
    delay(1000); // Wait 2 seconds between batch animations

    // Form string transitions for testing
    char fromStr[4];
    char toStr[4];
    
    snprintf(fromStr, sizeof(fromStr), "%02d", counter % 100);
    counter++;
    snprintf(toStr, sizeof(toStr), "%02d", counter % 100);

    Serial.printf("[MAIN LOOP] Animating Digits: '%s' -> '%s'\n", fromStr, toStr);

    // Task 1: Large font digit slide down (x=0, y=0, non-mini)
    animator.animate(0, 0, std::string(fromStr), std::string(toStr), false, 0);

    // Task 2: Mini font string slide down with a 100ms delay in the same batch (x=20, y=8, mini)
    std::string miniFrom = (counter % 2 == 1) ? "RUN" : "SET";
    std::string miniTo   = (counter % 2 == 1) ? "SET" : "RUN";
    animator.animate(20, 8, miniFrom, miniTo, true, 100);

    // Seal and execute batch concurrently
    animator.startAnimate();

    // Testing and verifying Timer output in main loop
    uint64_t ts = timerModule.getTimestamp();
    Timer::TimeData tData = timerModule.get();

    Serial.printf("[MAIN LOOP] Timestamp: %llu | Date: %04u-%02u-%02u %02u:%02u:%02u (DoW: %u)\n",
                  ts,
                  tData.year, tData.month, tData.day,
                  tData.hour, tData.minute, tData.second,
                  tData.dayOfWeek);
}