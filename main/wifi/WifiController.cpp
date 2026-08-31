#include "wifi/WifiController.h"

WifiController::~WifiController() {
    if (taskHandle != nullptr) {
        vTaskDelete(taskHandle);
    }
    if (mutex != nullptr) {
        vSemaphoreDelete(mutex);
    }
}

void WifiController::init() {
    mutex = xSemaphoreCreateMutex();
    WiFi.mode(WIFI_STA);

    loadFromNVS();

    // Start background monitor task on core 0 (1s check loop)
    xTaskCreatePinnedToCore(
        wifiTask,
        "WifiTask",
        4096,
        this,
        1,
        &taskHandle,
        0
    );
}

void WifiController::addWifi(std::string ssid, std::string password) {
    if (ssid.empty()) return;

    xSemaphoreTake(mutex, portMAX_DELAY);
    
    bool found = false;
    for (auto& w : savedWifis) {
        if (w.ssid == ssid) {
            w.password = password; // Update existing
            found = true;
            break;
        }
    }

    if (!found) {
        savedWifis.push_back({ssid, password});
    }

    saveToNVS();
    targetSsid = ssid;
    autoReconnectEnabled = true;

    xSemaphoreGive(mutex);

    // Trigger immediate connection attempt
    WiFi.disconnect();
}

void WifiController::removeWifi(std::string ssid) {
    xSemaphoreTake(mutex, portMAX_DELAY);

    for (auto it = savedWifis.begin(); it != savedWifis.end(); ++it) {
        if (it->ssid == ssid) {
            savedWifis.erase(it);
            break;
        }
    }

    saveToNVS();

    bool wasConnectedToThis = (WiFi.status() == WL_CONNECTED && WiFi.SSID().c_str() == ssid);
    xSemaphoreGive(mutex);

    if (wasConnectedToThis) {
        WiFi.disconnect();
    }
}

void WifiController::connect(std::string ssid) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    targetSsid = ssid;
    autoReconnectEnabled = true;
    xSemaphoreGive(mutex);

    WiFi.disconnect();
}

void WifiController::disconnect() {
    xSemaphoreTake(mutex, portMAX_DELAY);
    autoReconnectEnabled = false;
    targetSsid = "";
    xSemaphoreGive(mutex);

    WiFi.disconnect();
}

bool WifiController::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

std::string WifiController::getCurrentSSID() {
    if (isConnected()) {
        return std::string(WiFi.SSID().c_str());
    }
    return "";
}

void WifiController::loadFromNVS() {
    preferences.begin("wifis", true);
    int count = preferences.getInt("count", 0);

    savedWifis.clear();
    for (int i = 0; i < count; i++) {
        String sKey = "s" + String(i);
        String pKey = "p" + String(i);
        String s = preferences.getString(sKey.c_str(), "");
        String p = preferences.getString(pKey.c_str(), "");
        if (s.length() > 0) {
            savedWifis.push_back({s.c_str(), p.c_str()});
        }
    }
    preferences.end();
}

void WifiController::saveToNVS() {
    preferences.begin("wifis", false);
    preferences.clear();
    preferences.putInt("count", savedWifis.size());

    for (size_t i = 0; i < savedWifis.size(); i++) {
        String sKey = "s" + String(i);
        String pKey = "p" + String(i);
        preferences.putString(sKey.c_str(), savedWifis[i].ssid.c_str());
        preferences.putString(pKey.c_str(), savedWifis[i].password.c_str());
    }
    preferences.end();
}

void WifiController::wifiTask(void* pvParameters) {
    WifiController* self = static_cast<WifiController*>(pvParameters);

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        xSemaphoreTake(self->mutex, portMAX_DELAY);
        bool shouldReconnect = self->autoReconnectEnabled;
        std::string explicitTarget = self->targetSsid;
        auto wifis = self->savedWifis;
        xSemaphoreGive(self->mutex);

        if (!shouldReconnect || wifis.empty()) {
            continue;
        }

        if (WiFi.status() != WL_CONNECTED) {
            // Attempt 1: Targeted SSID if set
            if (!explicitTarget.empty()) {
                for (const auto& w : wifis) {
                    if (w.ssid == explicitTarget) {
                        WiFi.begin(w.ssid.c_str(), w.password.c_str());
                        
                        int timeout = 0;
                        while (WiFi.status() != WL_CONNECTED && timeout < 10) {
                            vTaskDelay(pdMS_TO_TICKS(500));
                            timeout++;
                        }
                        break;
                    }
                }
            }

            // Attempt 2: Sequential failover across saved networks
            if (WiFi.status() != WL_CONNECTED) {
                for (const auto& w : wifis) {
                    WiFi.begin(w.ssid.c_str(), w.password.c_str());
                    
                    int timeout = 0;
                    while (WiFi.status() != WL_CONNECTED && timeout < 10) {
                        vTaskDelay(pdMS_TO_TICKS(500));
                        timeout++;
                    }

                    if (WiFi.status() == WL_CONNECTED) {
                        break;
                    }
                }
            }
        }
    }
}