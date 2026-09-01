#pragma once

#include <string>
#include <cstdint>
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

private: 
    WifiData  internetWifi;
    WifiData  configWifi;
    int8_t    timezone;
    int64_t   timeOffset;
    SleepMode sleepMode;
    bool      openConfig;

    Preferences prefs;
    bool initialized;

    // Helper functions for reading/writing NVS
    void saveWifiData(const char* prefix, const WifiData& data);
    WifiData loadWifiData(const char* prefix);
    void saveSleepMode(const SleepMode& mode);
    SleepMode loadSleepMode();

public:
    Storer();

    void init();
    bool isInit() const;
    void save(); // Persists all current RAM state to NVS

    // Config WiFi
    void setConfigWifi(WifiData _configWifi);
    WifiData getConfigWifi() const;

    // Internet WiFi
    void setInternetWifi(WifiData _internetWifi);
    WifiData getInternetWifi() const;

    // Timezone
    void setTimezone(int8_t _timezone);
    int8_t getTimezone() const;

    // Time Offset
    void setTimeOffset(int64_t _timeOffset);
    int64_t getTimeOffset() const;

    // Sleep Mode
    void setSleepMode(SleepMode _sleepMode);
    SleepMode getSleepMode() const;

    // Open Config
    void setOpenConfig(bool _openConfig);
    bool getOpenConfig() const;
};