#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <string>
#include <vector>
#include <functional>

class Storer {
public:
    struct ConfigWifiData {
        bool enable = true;
        std::string ssid;
        std::string password;
    };

    struct WifiData {
        std::string ssid;
        std::string password;
    };

    struct TimestampData {
        int8_t timezone;
        int64_t timestampOffset; // in seconds
        uint64_t timestamp;       // in seconds
    };

    struct SchedulePeriod {
        enum class ClockMode : uint8_t {
            NORMAL = 0,
            MINI   = 1,
        };
        enum class RepeatState : uint8_t {
            MINUTE = 0,
            HOUR   = 1,
            DAY    = 2,
            WEEK   = 3,
        };
        struct SoundState {
            uint8_t volumn; // 0-100
            uint8_t repeat;
            uint8_t soundId;
        };

        RepeatState repeat;
        SoundState startSound;
        uint8_t startClockFlash;
        ClockMode clockMode;
    };

    using Callback = std::function<void()>;

private:
    Preferences prefs;
    const char* NVS_NAMESPACE = "esp_stater";

    ConfigWifiData configWifi;
    WifiData internetWifi;
    TimestampData timestampData;
    std::vector<SchedulePeriod> schedule;

    // Callbacks
    Callback configWifiCB;
    Callback internetWifiCB;
    Callback timestampDataCB;
    Callback scheduleCB;

public:
    Storer();
    ~Storer();

    void init();
    void save(); // Commits all current in-memory state to NVS

    // ConfigWifi
    void setConfigWifi(const ConfigWifiData& _configWifi);
    ConfigWifiData getConfigWifi() const;
    void onConfigWifiChange(Callback callback);

    // InternetWifi
    void setInternetWifi(const WifiData& _internetWifi);
    WifiData getInternetWifi() const;
    void onInternetWifiChange(Callback callback);

    // TimestampData
    void setTimestampData(const TimestampData& _timestampData);
    TimestampData getTimestampData() const;
    void onTimestampDataChange(Callback callback);

    // Schedule
    void setSchedule(const std::vector<SchedulePeriod>& _schedule);
    std::vector<SchedulePeriod> getSchedule() const;
    void onScheduleChange(Callback callback);
};