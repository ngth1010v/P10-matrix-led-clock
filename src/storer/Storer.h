#pragma one

#include <string>
#include <cstdint>
#include <functional>
#include <Preferences.h>

class Storer {
public:
    struct WifiData { 
        std::string name; 
        std::string password; 
    };

    struct TimeData {  
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
    };

    struct SleepMode {
        bool     enable;
        TimeData from;
        TimeData to;            
    };

    using Callback = std::function<void()>;

private: 
    WifiData  internetWifi;
    WifiData  configWifi;
    int8_t    timezone;
    int64_t   timeOffset;
    SleepMode sleepMode;

    Preferences prefs;

    // Callbacks
    Callback configWifiCb;
    Callback internetWifiCb;
    Callback timezoneCb;
    Callback timeOffsetCb;
    Callback sleepModeCb;

    // Helper functions for reading/writing NVS
    void saveWifiData(const char* prefix, const WifiData& data);
    WifiData loadWifiData(const char* prefix);
    void saveSleepMode(const SleepMode& mode);
    SleepMode loadSleepMode();

public:
    Storer();

    void init();
    void save(); // Persists all current RAM state to NVS

    // Config WiFi
    void setConfigWifi(WifiData _configWifi);
    WifiData getConfigWifi() const;
    void onConfigWifiChange(Callback callback);

    // Internet WiFi
    void setInternetWifi(WifiData _internetWifi);
    WifiData getInternetWifi() const;
    void onInternetWifiChange(Callback callback);

    // Timezone
    void setTimezone(int8_t _timezone);
    int8_t getTimezone() const;
    void onTimezoneChange(Callback callback);

    // Time Offset
    void setTimeOffset(int64_t _timeOffset);
    int64_t getTimeOffset() const;
    void onTimeOffsetChange(Callback callback);

    // Sleep Mode
    void setSleepMode(SleepMode _sleepMode);
    SleepMode getSleepMode() const;
    void onSleepModeChange(Callback callback);
};
