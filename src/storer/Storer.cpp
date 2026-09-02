#include "Storer.h"

Storer::Storer()
    : timezone(0),
      timeOffset(0),
      sleepMode{false, {0, 0, 0}, {0, 0, 0}},
      openConfig(false),
      initialized(false) {}

void Storer::init() {
    prefs.begin("stater", false); // Open Preferences namespace "stater" in RW mode

    // Load persisted values from NVS into RAM
    configWifi   = loadWifiData("cfg");
    internetWifi = loadWifiData("net");
    timezone     = prefs.getChar("tz", 0);
    timeOffset   = prefs.getLong64("t_off", 0);
    sleepMode    = loadSleepMode();
    openConfig   = prefs.getBool("op_cfg", true);

    initialized = true;

    Serial.println("[Storer] Initialized");
    logData();
}

bool Storer::isInit() const {
    return initialized;
}

void Storer::save() {
    // Commit all current RAM state to NVS
    saveWifiData("cfg", configWifi);
    saveWifiData("net", internetWifi);
    prefs.putChar("tz", timezone);
    prefs.putLong64("t_off", timeOffset);
    saveSleepMode(sleepMode);
    prefs.putBool("op_cfg", openConfig);
}

// Helper methods for persistent NVS storage
void Storer::saveWifiData(const char* prefix, const WifiData& data) {
    std::string keyName = std::string(prefix) + "_n";
    std::string keyPass = std::string(prefix) + "_p";

    prefs.putString(keyName.c_str(), data.name.c_str());
    prefs.putString(keyPass.c_str(), data.password.c_str());
}

Storer::WifiData Storer::loadWifiData(const char* prefix) {
    std::string keyName = std::string(prefix) + "_n";
    std::string keyPass = std::string(prefix) + "_p";

    WifiData data;
    data.name     = prefs.getString(keyName.c_str(), "").c_str();
    data.password = prefs.getString(keyPass.c_str(), "").c_str();

    return data;
}

void Storer::saveSleepMode(const SleepMode& mode) {
    prefs.putBool("sm_en", mode.enable);

    prefs.putUChar("sm_fh", mode.from.hour);
    prefs.putUChar("sm_fm", mode.from.minute);
    prefs.putUChar("sm_fs", mode.from.second);

    prefs.putUChar("sm_th", mode.to.hour);
    prefs.putUChar("sm_tm", mode.to.minute);
    prefs.putUChar("sm_ts", mode.to.second);
}

Storer::SleepMode Storer::loadSleepMode() {
    SleepMode mode;

    mode.enable      = prefs.getBool("sm_en", false);

    mode.from.hour   = prefs.getUChar("sm_fh", 0);
    mode.from.minute = prefs.getUChar("sm_fm", 0);
    mode.from.second = prefs.getUChar("sm_fs", 0);

    mode.to.hour     = prefs.getUChar("sm_th", 0);
    mode.to.minute   = prefs.getUChar("sm_tm", 0);
    mode.to.second   = prefs.getUChar("sm_ts", 0);

    return mode;
}

void Storer::logData() const {
    Serial.println("========== STORER DATA ==========");

    Serial.println("[Config WiFi]");
    Serial.printf("  Name: %s\n", configWifi.name.c_str());
    Serial.printf("  Password: %s\n", configWifi.password.empty() ? "(empty)" : "********");

    Serial.println("[Internet WiFi]");
    Serial.printf("  Name: %s\n", internetWifi.name.c_str());
    Serial.printf("  Password: %s\n", internetWifi.password.empty() ? "(empty)" : "********");

    Serial.println("[Timezone]");
    Serial.printf("  Timezone: %d\n", timezone);

    Serial.println("[Time Offset]");
    Serial.printf("  Time Offset: %lld\n", timeOffset);

    Serial.println("[Sleep Mode]");
    Serial.printf("  Enable: %s\n", sleepMode.enable ? "true" : "false");

    Serial.printf(
        "  From: %02u:%02u:%02u\n",
        sleepMode.from.hour,
        sleepMode.from.minute,
        sleepMode.from.second
    );

    Serial.printf(
        "  To:   %02u:%02u:%02u\n",
        sleepMode.to.hour,
        sleepMode.to.minute,
        sleepMode.to.second
    );

    Serial.println("[Open Config]");
    Serial.printf("  Open Config: %s\n", openConfig ? "true" : "false");

    Serial.println("=================================");
}

// Config WiFi
void Storer::setConfigWifi(WifiData _configWifi) {
    configWifi = _configWifi;
}

Storer::WifiData Storer::getConfigWifi() const {
    return configWifi;
}

// Internet WiFi
void Storer::setInternetWifi(WifiData _internetWifi) {
    internetWifi = _internetWifi;
}

Storer::WifiData Storer::getInternetWifi() const {
    return internetWifi;
}

// Timezone
void Storer::setTimezone(int8_t _timezone) {
    timezone = _timezone;
}

int8_t Storer::getTimezone() const {
    return timezone;
}

// Time Offset
void Storer::setTimeOffset(int64_t _timeOffset) {
    timeOffset = _timeOffset;
}

int64_t Storer::getTimeOffset() const {
    return timeOffset;
}

// Sleep Mode
void Storer::setSleepMode(SleepMode _sleepMode) {
    sleepMode = _sleepMode;
}

Storer::SleepMode Storer::getSleepMode() const {
    return sleepMode;
}

// Open Config
void Storer::setOpenConfig(bool _openConfig) {
    openConfig = _openConfig;
}

bool Storer::getOpenConfig() const {
    return openConfig;
}