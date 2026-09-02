#include "ConfigController.h"
#include "web/index.h"
#include "web/style.h"
#include "web/script.h"

ConfigController::ConfigController() : server(80) {}

ConfigController::~ConfigController() {
    if (buttonTaskHandle != nullptr) {
        vTaskDelete(buttonTaskHandle);
    }
}

void ConfigController::init(Storer* storer) {
    Serial.println();
    Serial.println("========== ConfigController::init ==========");

    if (storer == nullptr) {
        Serial.println("[ConfigController] ERROR: storer == nullptr");
        return;
    }

    if (!storer->isInit()) {
        Serial.println("[ConfigController] ERROR: Storer is not initialized!");
        return;
    }

    Serial.println("[ConfigController] Storer OK.");

    this->storer = storer;

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    Serial.print("[ConfigController] Button GPIO: ");
    Serial.println(BUTTON_PIN);

    Serial.println("[ConfigController] Creating ButtonTask...");

    BaseType_t taskResult = xTaskCreate(
        ConfigController::buttonTask,
        "ButtonTask",
        4096,
        this,
        1,
        &buttonTaskHandle
    );

    Serial.print("[ConfigController] ButtonTask creation: ");
    Serial.println(
        taskResult == pdPASS
            ? "SUCCESS"
            : "FAILED"
    );

    bool openConfig = storer->getOpenConfig();

    Serial.print("[ConfigController] getOpenConfig(): ");
    Serial.println(openConfig ? "TRUE" : "FALSE");

    if (openConfig) {
        Serial.println("[ConfigController] -> Starting AP server...");
        startServer();
    } else {
        Serial.println("[ConfigController] -> Config server disabled.");
        stopServer();
    }

    Serial.println("[ConfigController] Initialized successfully.");
    Serial.println("============================================");
}

void ConfigController::startServer() {
    Serial.println();
    Serial.println("========== ConfigController::startServer ==========");

    if (serverRunning) {
        Serial.println("[ConfigController] Server already running.");
        return;
    }

    Serial.println("[ConfigController] Step 1: Loading AP config...");

    Storer::WifiData apConfig = storer->getConfigWifi();

    String ssid = apConfig.name.empty() ? DEFAULT_AP_SSID : apConfig.name.c_str();
    String pass = apConfig.password.empty() ? DEFAULT_AP_PASS : apConfig.password.c_str();

    Serial.println("[ConfigController] Step 2: Setting WiFi mode -> WIFI_AP");

    // Force AP mode only during setup to eliminate STA channel-hopping packet drops
    WiFi.mode(WIFI_AP);
    delay(100);

    // Explicitly configure AP IP Subnet
    IPAddress local_IP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_IP, gateway, subnet);

    bool apStarted = WiFi.softAP(ssid.c_str(), pass.c_str(), 1, false, 4);

    if (!apStarted) {
        Serial.println("[ConfigController] ERROR: SoftAP failed!");
        return;
    }

    // Start DNS Server (Catch-all for Captive Portal)
    Serial.println("[ConfigController] Step 4: Starting DNS server...");
    bool dnsStarted = dnsServer.start(53, "*", local_IP);

    // Register Routes
    Serial.println("[ConfigController] Step 5: Setting up HTTP routes...");
    setupRoutes();

    // Start Web Server
    Serial.println("[ConfigController] Step 6: Starting HTTP server...");
    server.begin();

    serverRunning = true;
    pendingStop = false;

    storer->setOpenConfig(true);
    storer->save();

    Serial.println("*************** CONFIG AP READY ****************");
    Serial.print("* IP         : "); Serial.println(WiFi.softAPIP());
    Serial.println("************************************************");
}


void ConfigController::stopServer() {
    Serial.println();
    Serial.println("========== ConfigController::stopServer ==========");

    if (!serverRunning) {
        Serial.println("[ConfigController] Server is already stopped.");
        Serial.println("====================================================");
        return;
    }

    Serial.println("[ConfigController] Stopping DNS...");
    dnsServer.stop();
    Serial.println("[ConfigController] DNS stopped.");

    Serial.println("[ConfigController] Stopping HTTP server...");
    server.end();
    Serial.println("[ConfigController] HTTP server stopped.");

    Serial.println("[ConfigController] Disconnecting SoftAP...");
    WiFi.softAPdisconnect(true);
    Serial.println("[ConfigController] SoftAP disconnected.");

    // Return to WIFI_STA mode (do not turn WiFi OFF completely)
    Serial.println("[ConfigController] Setting WiFi mode -> WIFI_STA");
    WiFi.mode(WIFI_STA);

    serverRunning = false;
    pendingStop = false;

    Serial.println("[ConfigController] Server stopped.");

    storer->setOpenConfig(false);
    storer->save();

    Serial.println("====================================================");
}

void ConfigController::setupRoutes() {
    // Serve plain HTML from PROGMEM
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html);
    });

    // Serve plain CSS from PROGMEM
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/css", style_css);
    });

    // Serve plain JS from PROGMEM
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "application/javascript", script_js);
    });

    // Captive Portal Handlers
    server.on("/generate_204", [](AsyncWebServerRequest *request) { request->redirect("/"); });
    server.on("/redirect", [](AsyncWebServerRequest *request) { request->redirect("/"); });
    server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) { request->redirect("/"); });
    server.on("/canonical.html", [](AsyncWebServerRequest *request) { request->redirect("/"); });

    // GET /api/config
    server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(1024);

        Storer::WifiData internetWifi = storer->getInternetWifi();
        Storer::WifiData configWifi = storer->getConfigWifi();
        Storer::SleepMode sleepMode = storer->getSleepMode();

        doc["internetWifi"]["name"] = internetWifi.name;
        doc["internetWifi"]["password"] = internetWifi.password;
        doc["configWifi"]["name"] = configWifi.name;
        doc["configWifi"]["password"] = configWifi.password;
        doc["timezone"] = storer->getTimezone();
        doc["timeOffset"] = storer->getTimeOffset();

        JsonObject sm = doc.createNestedObject("sleepMode");
        sm["enable"] = sleepMode.enable;
        sm["from"]["hour"] = sleepMode.from.hour;
        sm["from"]["minute"] = sleepMode.from.minute;
        sm["from"]["second"] = sleepMode.from.second;
        sm["to"]["hour"] = sleepMode.to.hour;
        sm["to"]["minute"] = sleepMode.to.minute;
        sm["to"]["second"] = sleepMode.to.second;

        String responseStr;
        serializeJson(doc, responseStr);
        request->send(200, "application/json", responseStr);
    });

    // POST /api/config
    AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler(
        "/api/config", 
        [this](AsyncWebServerRequest *request, JsonVariant &json) {
            JsonObject jsonObj = json.as<JsonObject>();

            if (jsonObj.containsKey("internetWifi")) {
                Storer::WifiData net;
                net.name = jsonObj["internetWifi"]["name"] | "";
                net.password = jsonObj["internetWifi"]["password"] | "";
                storer->setInternetWifi(net);
            }

            if (jsonObj.containsKey("configWifi")) {
                Storer::WifiData cfg;
                cfg.name = jsonObj["configWifi"]["name"] | "";
                cfg.password = jsonObj["configWifi"]["password"] | "";
                storer->setConfigWifi(cfg);
            }

            if (jsonObj.containsKey("timezone")) {
                storer->setTimezone(jsonObj["timezone"].as<int8_t>());
            }

            if (jsonObj.containsKey("timeOffset")) {
                storer->setTimeOffset(jsonObj["timeOffset"].as<int64_t>());
            }

            if (jsonObj.containsKey("sleepMode")) {
                JsonObject smObj = jsonObj["sleepMode"];
                Storer::SleepMode sm;
                sm.enable = smObj["enable"] | false;
                sm.from.hour = smObj["from"]["hour"] | 0;
                sm.from.minute = smObj["from"]["minute"] | 0;
                sm.from.second = smObj["from"]["second"] | 0;
                sm.to.hour = smObj["to"]["hour"] | 0;
                sm.to.minute = smObj["to"]["minute"] | 0;
                sm.to.second = smObj["to"]["second"] | 0;
                storer->setSleepMode(sm);
            }

            storer->setOpenConfig(false);
            storer->save();

            request->send(200, "application/json", "{\"status\":\"success\"}");
            this->pendingStop = true;
        }
    );

    server.addHandler(handler);

    server.onNotFound([](AsyncWebServerRequest *request) {
        request->redirect("/");
    });
}



void ConfigController::buttonTask(void* parameter) {
    ConfigController* controller = static_cast<ConfigController*>(parameter);

    while (true) {
        if (controller->storer != nullptr && !controller->serverRunning) {
            if (digitalRead(BUTTON_PIN) == LOW) {
                vTaskDelay(pdMS_TO_TICKS(50));
                if (digitalRead(BUTTON_PIN) == LOW) {
                    Serial.println("[ConfigController] Button pressed. Opening AP server...");
                    controller->startServer();

                    while (digitalRead(BUTTON_PIN) == LOW) {
                        vTaskDelay(pdMS_TO_TICKS(50));
                    }
                }
            }
        }
        
        if (controller->serverRunning) {
            controller->dnsServer.processNextRequest();

            // Handle delayed shutdown safely outside of HTTP handler context
            if (controller->pendingStop) {
                vTaskDelay(pdMS_TO_TICKS(500)); // Allow response packet to be sent to client
                controller->stopServer();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}