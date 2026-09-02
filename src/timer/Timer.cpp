#include "Timer.h"

const char* Timer::NTP_SERVER_1 = "pool.ntp.org";
const char* Timer::NTP_SERVER_2 = "time.nist.gov";
const char* Timer::NTP_SERVER_3 = "time.google.com";
const char* Timer::NTP_SERVER_4 = "0.pool.ntp.org";
const char* Timer::NTP_SERVER_5 = "1.pool.ntp.org";

Timer::Timer() {}

Timer::~Timer() {
    if (ntpTaskHandle != nullptr) {
        vTaskDelete(ntpTaskHandle);
    }
}

void Timer::init(Storer* storer) {
    Serial.println();
    Serial.println("========== Timer::init ==========");

    if (storer == nullptr || !storer->isInit()) {
        Serial.println("[Timer] ERROR: Storer is null or not initialized!");
        return;
    }

    this->storer = storer;

    // Configure SNTP base settings (UTC base offset 0)
    configTime(0, 0, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);

    // Register callback for WiFi configuration changes to trigger immediate reconnect and sync
    this->storer->onInternetWifiChange([this](const Storer::WifiData& wifiData) {
        Serial.println("[Timer] WiFi credentials changed. Requesting immediate reconnect and sync...");
        if (ntpTaskHandle != nullptr) {
            xTaskNotifyGive(ntpTaskHandle);
        }
    });

    // Create background sync task on Core 0
    BaseType_t result = xTaskCreatePinnedToCore(
        Timer::ntpTask,
        "NtpSyncTask",
        4096,
        this,
        1,
        &ntpTaskHandle,
        0 // Core 0
    );

    Serial.print("[Timer] NtpSyncTask creation: ");
    Serial.println(result == pdPASS ? "SUCCESS" : "FAILED");
    Serial.println("=================================");
}

bool Timer::connectWifi() {
    if (storer == nullptr) return false;

    Storer::WifiData netConfig = storer->getInternetWifi();
    if (netConfig.name.empty()) {
        Serial.println("[Timer] Internet WiFi SSID is empty. Skipping connection.");
        return false;
    }

    // Already connected to target network
    if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == netConfig.name.c_str()) {
        return true;
    }

    Serial.printf("[Timer] Connecting to WiFi: %s...\n", netConfig.name.c_str());

    // Preserve AP mode if ConfigController active, otherwise use standard STA mode
    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
        WiFi.mode(WIFI_AP_STA);
    } else {
        WiFi.mode(WIFI_STA);
    }

    WiFi.begin(netConfig.name.c_str(), netConfig.password.c_str());

    // Non-blocking wait: 10-second timeout
    uint32_t startMs = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startMs < 10000)) {
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("[Timer] Connected to WiFi! IP: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("[Timer] WiFi connection timeout.");
        return false;
    }
}

void Timer::ntpTask(void* parameter) {
    Timer* timer = static_cast<Timer*>(parameter);

    while (true) {
        // Step 1: Ensure WiFi connection managed directly by Timer
        if (timer->connectWifi()) {
            // Step 2: Sync NTP
            if (timer->syncNTP()) {
                Serial.println("[Timer] Background NTP sync successful.");
            } else {
                Serial.println("[Timer] Background NTP sync failed. Retrying next cycle.");
            }
        } else {
            Serial.println("[Timer] WiFi unavailable. Using local POSIX timer.");
        }

        // Wait 60 seconds OR until woken by WiFi credential update notification
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(60000));
    }
}

bool Timer::syncNTP() {
    Serial.println("[Timer] Fetching NTP time...");

    // Re-initialize configTime with fallback servers
    configTime(0, 0, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);

    struct tm timeinfo;
    // Wait up to 5 seconds for SNTP sync confirmation
    if (getLocalTime(&timeinfo, 5000)) {
        return true;
    }

    // Try alternate fallback server pool if initial pool failed
    configTime(0, 0, NTP_SERVER_4, NTP_SERVER_5, nullptr);
    return getLocalTime(&timeinfo, 3000);
}

uint64_t Timer::getTimestamp() {
    time_t nowSec;
    time(&nowSec); // Base UTC epoch timestamp from internal RTC / POSIX clock

    int8_t tzHours = (storer != nullptr) ? storer->getTimezone() : 0;
    int64_t timeOffsetSec = (storer != nullptr) ? storer->getTimeOffset() : 0;

    int64_t totalSeconds = (int64_t)nowSec + ((int64_t)tzHours * 3600) + timeOffsetSec;

    return (totalSeconds < 0) ? 0 : (uint64_t)totalSeconds;
}

Timer::TimeData Timer::get() {
    uint64_t totalSec = getTimestamp();
    time_t rawTime = (time_t)totalSec;

    struct tm timeinfo;
    gmtime_r(&rawTime, &timeinfo); // Parse adjusted epoch using UTC calendar breakdown

    TimeData data;
    data.second    = static_cast<uint8_t>(timeinfo.tm_sec);
    data.minute    = static_cast<uint8_t>(timeinfo.tm_min);
    data.hour      = static_cast<uint8_t>(timeinfo.tm_hour);
    data.dayOfWeek = static_cast<uint8_t>(timeinfo.tm_wday);
    data.day       = static_cast<uint8_t>(timeinfo.tm_mday);
    data.month     = static_cast<uint8_t>(timeinfo.tm_mon + 1); // 0-indexed to 1-indexed
    data.year      = static_cast<uint32_t>(timeinfo.tm_year + 1900);

    return data;
}