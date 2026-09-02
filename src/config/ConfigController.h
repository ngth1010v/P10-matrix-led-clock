#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include "storer/Storer.h"
#include "wifi/WifiController.h"

class ConfigController {
private:
    static const uint8_t BUTTON_PIN = 17;
    const char* DEFAULT_AP_SSID = "Schedule-lock-config";
    const char* DEFAULT_AP_PASS = "11111111";

    Storer* storer = nullptr;
    WifiController* wifiController = nullptr;
    AsyncWebServer server;
    DNSServer dnsServer;
    TaskHandle_t buttonTaskHandle = nullptr;

    bool serverRunning = false;
    bool pendingStop = false;

    void startServer();
    void stopServer();
    void setupRoutes();
    
    static void buttonTask(void* parameter);

public:
    ConfigController();
    ~ConfigController();

    void init(Storer* storer, WifiController* wifiCtrl = nullptr);
};