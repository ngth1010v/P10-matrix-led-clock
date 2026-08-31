#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include <WiFi.h>
#include <Preferences.h>
#include <vector>
#include <string>

class WifiController {
private:
    struct WifiData {
        std::string ssid;
        std::string password;
    };

    std::vector<WifiData> savedWifis;
    Preferences preferences;
    
    TaskHandle_t taskHandle = nullptr;
    SemaphoreHandle_t mutex = nullptr;
    
    bool autoReconnectEnabled = true;
    std::string targetSsid = ""; // Explicit target requested by connect()

    void loadFromNVS();
    void saveToNVS();
    static void wifiTask(void* pvParameters);

public:
    WifiController() = default;
    ~WifiController();

    void init();
    void addWifi(std::string ssid, std::string password); // Add/update & try to connect
    void removeWifi(std::string ssid);                    // Forget & failover if active
    void connect(std::string ssid);                      // Force connect to saved network
    void disconnect();                                   // Disconnect & disable auto-reconnect
    
    bool isConnected();
    std::string getCurrentSSID();
};

#endif // WIFI_CONTROLLER_H