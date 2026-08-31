#include "timer/Timer.h"

Timer::~Timer() {
    if (taskHandle != nullptr) {
        vTaskDelete(taskHandle);
    }
    if (mutex != nullptr) {
        vSemaphoreDelete(mutex);
    }
}

void Timer::init() {
    mutex = xSemaphoreCreateMutex();

    // Start background sync task on core 1
    xTaskCreatePinnedToCore(
        timerTask,
        "TimerNTPTask",
        4096,
        this,
        1,
        &taskHandle,
        1
    );
}

bool Timer::syncNTP() {
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    // Try each of the 5 servers sequentially for fallback
    for (int i = 0; i < 5; i++) {
        configTime(0, 0, ntpServers[i]);
        
        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 3000)) { // 3s timeout
            return true;
        }
    }
    return false;
}

void Timer::timerTask(void* pvParameters) {
    Timer* self = static_cast<Timer*>(pvParameters);
    
    TickType_t lastSyncTick = 0;
    bool initialSynced = false;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        if (WiFi.status() == WL_CONNECTED) {
            TickType_t currentTick = xTaskGetTickCount();
            
            // Sync on initial boot connection OR every 60 minutes (3,600,000 ms)
            if (!initialSynced || (currentTick - lastSyncTick >= pdMS_TO_TICKS(3600000))) {
                if (self->syncNTP()) {
                    initialSynced = true;
                    lastSyncTick = currentTick;
                }
            }
        }
    }
}

void Timer::setTimezone(int8_t tz) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    timezone = tz;
    xSemaphoreGive(mutex);
}

void Timer::setTimestampOffset(int offset) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    timestampOffset = offset;
    xSemaphoreGive(mutex);
}

int Timer::getTimestampOffset() {
    xSemaphoreTake(mutex, portMAX_DELAY);
    int offset = timestampOffset;
    xSemaphoreGive(mutex);
    return offset;
}

uint32_t Timer::getTimestamp() {
    xSemaphoreTake(mutex, portMAX_DELAY);
    int8_t tz = timezone;
    int offset = timestampOffset;
    xSemaphoreGive(mutex);

    struct timeval tv;
    gettimeofday(&tv, nullptr);

    // Converts POSIX seconds + microseconds to ms
    uint64_t currentMs = ((uint64_t)tv.tv_sec * 1000ULL) + (tv.tv_usec / 1000);
    
    // Formula: UTC timestamp + (timezone * 3600000) + timestampOffset
    int64_t transformedMs = (int64_t)currentMs + ((int64_t)tz * 3600000LL) + (int64_t)offset;

    return (uint32_t)(transformedMs & 0xFFFFFFFF);
}

Timer::TimeData Timer::get() {
    xSemaphoreTake(mutex, portMAX_DELAY);
    int8_t tz = timezone;
    int offset = timestampOffset;
    xSemaphoreGive(mutex);

    struct timeval tv;
    gettimeofday(&tv, nullptr);

    // Apply timezone and offset conversion to epoch seconds
    int64_t rawMs = ((int64_t)tv.tv_sec * 1000LL) + (tv.tv_usec / 1000);
    int64_t transformedMs = rawMs + ((int64_t)tz * 3600000LL) + (int64_t)offset;

    time_t transformedSec = (time_t)(transformedMs / 1000LL);
    uint32_t subMili = (uint32_t)(transformedMs % 1000LL);

    struct tm timeinfo;
    gmtime_r(&transformedSec, &timeinfo);

    TimeData data;
    data.mili      = subMili;
    data.second    = timeinfo.tm_sec;
    data.minute    = timeinfo.tm_min;
    data.hour      = timeinfo.tm_hour;
    data.dayOfWeek = timeinfo.tm_wday;
    data.day       = timeinfo.tm_mday;
    data.month     = timeinfo.tm_mon + 1;
    data.year      = timeinfo.tm_year + 1900;

    return data;
}