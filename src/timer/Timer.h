#ifndef TIMER_H
#define TIMER_H

#include <WiFi.h>
#include <time.h>
#include <sys/time.h>

class Timer {
public:
    struct TimeData {
        uint32_t mili;     // ms (0 - 999)
        uint8_t  second;   // 0 - 59
        uint8_t  minute;   // 0 - 59
        uint8_t  hour;     // 0 - 23
        uint8_t  dayOfWeek;// 0 - 6 (0 = Sunday)
        uint8_t  day;      // 1 - 31
        uint8_t  month;    // 1 - 12
        uint32_t year;     // e.g., 2026
    };

private:
    int8_t timezone = 0;       // UTC offset in hours (-12 to +14)
    int timestampOffset = 0;   // Custom ms offset

    TaskHandle_t taskHandle = nullptr;
    SemaphoreHandle_t mutex = nullptr;

    const char* ntpServers[5] = {
        "pool.ntp.org",
        "time.nist.gov",
        "time.google.com",
        "time.cloudflare.com",
        "0.pool.ntp.org"
    };

    bool syncNTP();
    static void timerTask(void* pvParameters);

public:
    Timer() = default;
    ~Timer();

    void init();

    TimeData get();
    uint32_t getTimestamp(); // Adjusted epoch timestamp in ms

    void setTimezone(int8_t tz);
    void setTimestampOffset(int offset);
    int getTimestampOffset();
};

#endif // TIMER_H