#pragma once

#include <Arduino.h>
#include <time.h>
#include <sys/time.h>
#include <WiFi.h>
#include "storer/Storer.h"

class Timer {
public:
    struct TimeData {
        uint8_t  second;    // 0 - 59
        uint8_t  minute;    // 0 - 59
        uint8_t  hour;      // 0 - 23
        uint8_t  dayOfWeek; // 0 - 6 (0 = Sunday)
        uint8_t  day;       // 1 - 31
        uint8_t  month;     // 1 - 12
        uint32_t year;      // e.g., 2026
    };

private:
    Storer* storer = nullptr;
    TaskHandle_t ntpTaskHandle = nullptr;

    static const char* NTP_SERVER_1;
    static const char* NTP_SERVER_2;
    static const char* NTP_SERVER_3;
    static const char* NTP_SERVER_4;
    static const char* NTP_SERVER_5;

    static void ntpTask(void* parameter);
    bool connectWifi();
    bool syncNTP();

    const uint8_t localOffset = 1;

public:
    Timer();
    ~Timer();

    void init(Storer* storer);

    uint64_t getTimestamp(); // in seconds, factors in timezone + timeOffset
    TimeData get();          // factors in timezone + timeOffset
};