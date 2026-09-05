#include <Arduino.h>
#include <string>

#include "display/P10Driver.h"
#include "display/FontRenderer.h"

#include "storer/Storer.h"
#include "config/ConfigController.h"
#include "timer/Timer.h"
#include "display/Animator.h"
#include "buzzer/Buzzer.h"


// Module Instances
Storer storer;
ConfigController configController;

Buzzer buzzer;
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
    buzzer.init();

    storer.init();
    configController.init(&storer);

    // Initialize Timer module
    timerModule.init(&storer);

    p10Driver.init();
    fontRenderer.init();

    // Initialize Animator module
    animator.init(&p10Driver, &fontRenderer);
    
    Serial.println("[SETUP] Setup completed. Starting animation sequence in loop...");
}


std::string currentTimeString = "        ";
std::string currentDayOfWeekString = "";
std::string currentDateString = "     ";
std::string currentMiniClockString = "        ";

void loop() {
    delay(1000);

    // Mode:
    bool sleepModeOn = false;
    {
        const Storer::SleepMode sleepMode = storer.getSleepMode();

        if (sleepMode.enable) {
            constexpr unsigned int DAY_SECONDS = 24 * 60 * 60;

            const unsigned int fromDayTs =
                sleepMode.from.hour * 60 * 60 +
                sleepMode.from.minute * 60 +
                sleepMode.from.second;

            const unsigned int toDayTs =
                sleepMode.to.hour * 60 * 60 +
                sleepMode.to.minute * 60 +
                sleepMode.to.second;

            const unsigned int currentDayTs =
                timerModule.getTimestamp() % DAY_SECONDS;

            if (fromDayTs < toDayTs) {
                // Normal: e.g. 06:00 -> 22:00
                sleepModeOn =
                    fromDayTs <= currentDayTs &&
                    currentDayTs < toDayTs;
            }
            else if (fromDayTs > toDayTs) {
                // Cross midnight: e.g. 22:00 -> 06:00
                sleepModeOn =
                    currentDayTs >= fromDayTs ||
                    currentDayTs < toDayTs;
            }
            else {
                // from == to -> no active interval
                sleepModeOn = false;
            }
        }
    }

    // Sleep mode off
    if (!sleepModeOn) {
        Timer::TimeData tData = timerModule.get();

        // Time clock: HHMMSS
        {
            char buffer[9];

            snprintf(
                buffer,
                sizeof(buffer),
                "%02u:%02u:%02u",
                tData.hour,
                tData.minute,
                tData.second
            );

            const std::string nextTimeString(buffer);

            auto animateDigit = [&](uint x, char from, char to, uint delay) {
                if (from != to) {
                    animator.animate(
                        x,
                        0,
                        std::string(1, from),
                        std::string(1, to),
                        false,
                        delay
                    );
                }
            };

            // Hour
            animateDigit(0,  currentTimeString[0], nextTimeString[0], 500);
            animateDigit(6,  currentTimeString[1], nextTimeString[1], 400);

            animateDigit(13, currentTimeString[2], nextTimeString[2], 350);
            
            animateDigit(16, currentTimeString[3], nextTimeString[3], 300);
            animateDigit(22, currentTimeString[4], nextTimeString[4], 200);
            
            animateDigit(29, currentTimeString[5], nextTimeString[5], 250);
            
            animateDigit(32, currentTimeString[6], nextTimeString[6], 100);
            animateDigit(38, currentTimeString[7], nextTimeString[7], 0);

            currentTimeString = nextTimeString;
        }

        // Day of week
        {
            const char* daysOfWeek[] = {"S U N", "M O N", "T U E", "W E D", "T H U", "F R I", "S A T"};
            std::string nextDayOfWeek = (tData.dayOfWeek < 7) ? daysOfWeek[tData.dayOfWeek] : "---";

            if (currentDayOfWeekString != nextDayOfWeek){
                animator.animate(
                    64,
                    0,
                    currentDayOfWeekString,
                    nextDayOfWeek,
                    true,
                    0,
                    true,
                    false
                );   
                
                currentDayOfWeekString = nextDayOfWeek;
            }
        }

        // Date (dd/mm)
        {
            char buffer[6];

            snprintf(
                buffer,
                sizeof(buffer),
                "%02u/%02u",
                tData.day,
                tData.month
            );

            const std::string nextDateString(buffer);

            auto animateDigit = [&](uint x, char from, char to, uint delay) {
                if (from != to) {
                    animator.animate(
                        x,
                        11,
                        std::string(1, from),
                        std::string(1, to),
                        true,
                        delay
                    );
                }
            };

            animateDigit(46, currentDateString[0], nextDateString[0], 300);
            animateDigit(50, currentDateString[1], nextDateString[1], 200);

            animateDigit(54, currentDateString[2], nextDateString[2], 300);
            
            animateDigit(57, currentDateString[3], nextDateString[3], 100);
            animateDigit(61, currentDateString[4], nextDateString[4], 0);

            currentDateString = nextDateString;
        }

        // CLEAR: Mini clock
        {
            const std::string nextMiniClockString = "        ";

            auto animateDigit = [&](uint x, char from, char to, uint delay) {
                if (from != to) {
                    animator.animate(
                        x,
                        11,
                        std::string(1, from),
                        std::string(1, to),
                        true,
                        delay
                    );
                }
            };

            // Hour
            animateDigit(0,  currentMiniClockString[0], nextMiniClockString[0], 500);
            animateDigit(4,  currentMiniClockString[1], nextMiniClockString[1], 400);

            animateDigit(8, currentMiniClockString[2], nextMiniClockString[2], 350);
            
            animateDigit(10, currentMiniClockString[3], nextMiniClockString[3], 300);
            animateDigit(14, currentMiniClockString[4], nextMiniClockString[4], 200);
            
            animateDigit(18, currentMiniClockString[5], nextMiniClockString[5], 350);
            
            animateDigit(20, currentMiniClockString[6], nextMiniClockString[6], 100);
            animateDigit(24, currentMiniClockString[7], nextMiniClockString[7], 0);

            currentMiniClockString = nextMiniClockString;
        }


    }
    
    // Sleep mode on
    if (sleepModeOn) {
        Timer::TimeData tData = timerModule.get();

        // CLEAR: Time clock: HHMMSS
        {
            const std::string nextTimeString("        ");

            auto animateDigit = [&](uint x, char from, char to, uint delay) {
                if (from != to) {
                    animator.animate(
                        x,
                        0,
                        std::string(1, from),
                        std::string(1, to),
                        false,
                        delay
                    );
                }
            };

            // Hour
            animateDigit(0,  currentTimeString[0], nextTimeString[0], 500);
            animateDigit(6,  currentTimeString[1], nextTimeString[1], 400);

            animateDigit(13, currentTimeString[2], nextTimeString[2], 350);
            
            animateDigit(16, currentTimeString[3], nextTimeString[3], 300);
            animateDigit(22, currentTimeString[4], nextTimeString[4], 200);
            
            animateDigit(29, currentTimeString[5], nextTimeString[5], 250);
            
            animateDigit(32, currentTimeString[6], nextTimeString[6], 100);
            animateDigit(38, currentTimeString[7], nextTimeString[7], 0);

            currentTimeString = nextTimeString;
        }

        // CLEAR: Day of week
        {
            std::string nextDayOfWeek = "   ";

            if (currentDayOfWeekString != nextDayOfWeek){
                animator.animate(
                    64,
                    0,
                    currentDayOfWeekString,
                    nextDayOfWeek,
                    true,
                    0,
                    true,
                    false
                );   
                
                currentDayOfWeekString = nextDayOfWeek;
            }
        }

        // CLEAR: Date (dd/mm)
        {
            const std::string nextDateString = "     ";

            auto animateDigit = [&](uint x, char from, char to, uint delay) {
                if (from != to) {
                    animator.animate(
                        x,
                        11,
                        std::string(1, from),
                        std::string(1, to),
                        true,
                        delay
                    );
                }
            };

            animateDigit(46, currentDateString[0], nextDateString[0], 300);
            animateDigit(50, currentDateString[1], nextDateString[1], 200);

            animateDigit(54, currentDateString[2], nextDateString[2], 300);
            
            animateDigit(57, currentDateString[3], nextDateString[3], 100);
            animateDigit(61, currentDateString[4], nextDateString[4], 0);

            currentDateString = nextDateString;
        }

        // Mini clock
        {
            char buffer[9];

            snprintf(
                buffer,
                sizeof(buffer),
                "%02u:%02u:%02u",
                tData.hour,
                tData.minute,
                tData.second
            );

            const std::string nextMiniClockString(buffer);

            auto animateDigit = [&](uint x, char from, char to, uint delay) {
                if (from != to) {
                    animator.animate(
                        x,
                        11,
                        std::string(1, from),
                        std::string(1, to),
                        true,
                        delay
                    );
                }
            };

            // Hour
            animateDigit(0,  currentMiniClockString[0], nextMiniClockString[0], 500);
            animateDigit(4,  currentMiniClockString[1], nextMiniClockString[1], 400);

            animateDigit(8, currentMiniClockString[2], nextMiniClockString[2], 350);
            
            animateDigit(10, currentMiniClockString[3], nextMiniClockString[3], 300);
            animateDigit(14, currentMiniClockString[4], nextMiniClockString[4], 200);
            
            animateDigit(18, currentMiniClockString[5], nextMiniClockString[5], 250);
            
            animateDigit(20, currentMiniClockString[6], nextMiniClockString[6], 100);
            animateDigit(24, currentMiniClockString[7], nextMiniClockString[7], 0);

            currentMiniClockString = nextMiniClockString;
        }

    }
    
    
    animator.startAnimate();
}