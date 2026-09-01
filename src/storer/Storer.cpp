#include "Storer.h"

Storer::Storer() {}

Storer::~Storer() {
    prefs.end();
}

void Storer::init() {
    prefs.begin(NVS_NAMESPACE, false);

    // Load ConfigWifi
    configWifi.enable = prefs.getBool("cfg_en", true);
    configWifi.ssid = prefs.getString("cfg_ssid", "").c_str();
    configWifi.password = prefs.getString("cfg_pass", "").c_str();

    // Load InternetWifi
    internetWifi.ssid = prefs.getString("net_ssid", "").c_str();
    internetWifi.password = prefs.getString("net_pass", "").c_str();

    // Load TimestampData
    timestampData.timezone = prefs.getChar("ts_tz", 0);
    timestampData.timestampOffset = prefs.getLong64("ts_offset", 0);
    timestampData.timestamp = prefs.getULong64("ts_val", 0);

    // Load Schedule Vector
    size_t schLen = prefs.getBytesLength("sch_vec");
    if (schLen > 0 && schLen % sizeof(SchedulePeriod) == 0) {
        size_t count = schLen / sizeof(SchedulePeriod);
        schedule.resize(count);
        prefs.getBytes("sch_vec", schedule.data(), schLen);
    } else {
        schedule.clear();
    }
}

void Storer::save() {
    prefs.begin(NVS_NAMESPACE, false);

    // Save ConfigWifi
    prefs.putBool("cfg_en", configWifi.enable);
    prefs.putString("cfg_ssid", configWifi.ssid.c_str());
    prefs.putString("cfg_pass", configWifi.password.c_str());

    // Save InternetWifi
    prefs.putString("net_ssid", internetWifi.ssid.c_str());
    prefs.putString("net_pass", internetWifi.password.c_str());

    // Save TimestampData
    prefs.putChar("ts_tz", timestampData.timezone);
    prefs.putLong64("ts_offset", timestampData.timestampOffset);
    prefs.putULong64("ts_val", timestampData.timestamp);

    // Save Schedule Vector
    if (!schedule.empty()) {
        prefs.putBytes("sch_vec", schedule.data(), schedule.size() * sizeof(SchedulePeriod));
    } else {
        prefs.remove("sch_vec");
    }
}

// ConfigWifi
void Storer::setConfigWifi(const ConfigWifiData& _configWifi) {
    configWifi = _configWifi;
    if (configWifiCB) configWifiCB();
}

Storer::ConfigWifiData Storer::getConfigWifi() const {
    return configWifi;
}

void Storer::onConfigWifiChange(Callback callback) {
    configWifiCB = callback;
}

// InternetWifi
void Storer::setInternetWifi(const WifiData& _internetWifi) {
    internetWifi = _internetWifi;
    if (internetWifiCB) internetWifiCB();
}

Storer::WifiData Storer::getInternetWifi() const {
    return internetWifi;
}

void Storer::onInternetWifiChange(Callback callback) {
    internetWifiCB = callback;
}

// TimestampData
void Storer::setTimestampData(const TimestampData& _timestampData) {
    timestampData = _timestampData;
    if (timestampDataCB) timestampDataCB();
}

Storer::TimestampData Storer::getTimestampData() const {
    return timestampData;
}

void Storer::onTimestampDataChange(Callback callback) {
    timestampDataCB = callback;
}

// Schedule
void Storer::setSchedule(const std::vector<SchedulePeriod>& _schedule) {
    schedule = _schedule;
    if (scheduleCB) scheduleCB();
}

std::vector<Storer::SchedulePeriod> Storer::getSchedule() const {
    return schedule;
}

void Storer::onScheduleChange(Callback callback) {
    scheduleCB = callback;
}