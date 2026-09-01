
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

// Config WiFi
void Storer::setConfigWifi(WifiData _configWifi) {
    if (configWifi.name != _configWifi.name ||
        configWifi.password != _configWifi.password) {

        configWifi = _configWifi;

        if (configWifiCb)
            configWifiCb();
    }
}

Storer::WifiData Storer::getConfigWifi() const {
    return configWifi;
}

void Storer::onConfigWifiChange(Callback callback) {
    configWifiCb = callback;

    if (initialized && configWifiCb)
        configWifiCb();
}

// Internet WiFi
void Storer::setInternetWifi(WifiData _internetWifi) {
    if (internetWifi.name != _internetWifi.name ||
        internetWifi.password != _internetWifi.password) {

        internetWifi = _internetWifi;

        if (internetWifiCb)
            internetWifiCb();
    }
}

Storer::WifiData Storer::getInternetWifi() const {
    return internetWifi;
}

void Storer::onInternetWifiChange(Callback callback) {
    internetWifiCb = callback;

    if (initialized && internetWifiCb)
        internetWifiCb();
}

// Timezone
void Storer::setTimezone(int8_t _timezone) {
    if (timezone != _timezone) {
        timezone = _timezone;

        if (timezoneCb)
            timezoneCb();
    }
}

int8_t Storer::getTimezone() const {
    return timezone;
}

void Storer::onTimezoneChange(Callback callback) {
    timezoneCb = callback;

    if (initialized && timezoneCb)
        timezoneCb();
}

// Time Offset
void Storer::setTimeOffset(int64_t _timeOffset) {
    if (timeOffset != _timeOffset) {
        timeOffset = _timeOffset;

        if (timeOffsetCb)
            timeOffsetCb();
    }
}

int64_t Storer::getTimeOffset() const {
    return timeOffset;
}

void Storer::onTimeOffsetChange(Callback callback) {
    timeOffsetCb = callback;

    if (initialized && timeOffsetCb)
        timeOffsetCb();
}

// Sleep Mode
void Storer::setSleepMode(SleepMode _sleepMode) {
    sleepMode = _sleepMode;

    if (sleepModeCb)
        sleepModeCb();
}

Storer::SleepMode Storer::getSleepMode() const {
    return sleepMode;
}

void Storer::onSleepModeChange(Callback callback) {
    sleepModeCb = callback;

    if (initialized && sleepModeCb)
        sleepModeCb();
}

// Open Config
void Storer::setOpenConfig(bool _openConfig) {
    if (openConfig != _openConfig) {
        openConfig = _openConfig;

        if (openConfigCb)
            openConfigCb();
    }
}

bool Storer::getOpenConfig() const {
    return openConfig;
}

void Storer::onOpenConfigChange(Callback callback) {
    openConfigCb = callback;

    if (initialized && openConfigCb)
        openConfigCb();
}
