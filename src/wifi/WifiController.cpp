#include "wifi/WifiController.h"
#include <esp_wifi.h>

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
    WiFi.setAutoReconnect(true);

    // Disable Wi-Fi power saving mode to eliminate RF interrupt jitter that impacts display timing
    esp_wifi_set_ps(WIFI_PS_NONE);

    loadFromNVS();

    // Start background monitor task on core 1
    xTaskCreatePinnedToCore(
        wifiTask,
        "WifiTask",
        4096,
        this,
        1,
        &taskHandle,
        1
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
    currentRetryIdx = 0;

    xSemaphoreGive(mutex);

    // Trigger connection attempt asynchronously
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
    currentRetryIdx = 0;
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
        vTaskDelay(pdMS_TO_TICKS(5000));

        xSemaphoreTake(self->mutex, portMAX_DELAY);
        bool shouldReconnect = self->autoReconnectEnabled;
        std::string explicitTarget = self->targetSsid;
        auto wifis = self->savedWifis;
        size_t retryIdx = self->currentRetryIdx;
        xSemaphoreGive(self->mutex);

        if (!shouldReconnect || wifis.empty()) {
            continue;
        }

        if (WiFi.status() != WL_CONNECTED) {
            std::string connectSsid = "";
            std::string connectPass = "";

            if (!explicitTarget.empty()) {
                for (const auto& w : wifis) {
                    if (w.ssid == explicitTarget) {
                        connectSsid = w.ssid;
                        connectPass = w.password;
                        break;
                    }
                }
            }

            if (connectSsid.empty()) {
                if (retryIdx >= wifis.size()) {
                    retryIdx = 0;
                }
                connectSsid = wifis[retryIdx].ssid;
                connectPass = wifis[retryIdx].password;

                xSemaphoreTake(self->mutex, portMAX_DELAY);
                self->currentRetryIdx = (retryIdx + 1) % wifis.size();
                xSemaphoreGive(self->mutex);
            }

            if (!connectSsid.empty()) {
                WiFi.begin(connectSsid.c_str(), connectPass.c_str());
            }
        } else {
            // Re-enforce modem power saving disabled when connected
            esp_wifi_set_ps(WIFI_PS_NONE);
        }
    }
}