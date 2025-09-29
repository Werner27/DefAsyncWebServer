#include "WebServerClass.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>
#include "html_template.h"
#include "html_pages.h"
#include <ConfigManager.h>

WebServerClass::WebServerClass()
: _server(80), _ws("/ws") {}

void WebServerClass::setCredentials(const String& ssid, const String& password) {
    _ssid = ssid;
    _password = password;
}

void WebServerClass::begin() {
    Serial.begin(115200);
    pinMode(_ledPin, OUTPUT);
    digitalWrite(_ledPin, LOW);

    if (!SPIFFS.begin(true)) {
        Serial.println("Fehler beim Mounten von SPIFFS");
    }

    ConfigManager::begin();
    if (_ssid.isEmpty() || _password.isEmpty()) {
        Serial.println("Keine WLAN-Daten gesetzt, lese aus ConfigManager...");
        loadEEPROMWifiConf(false);
        loadEEPROMWifiConf(true); // AP-Daten laden
    }
    connectOrStartAP();

    // Statische Assets
    _server.serveStatic("/favicon-96x96.png", SPIFFS, "/favicon-96x96.png");
    _server.serveStatic("/style.css",   SPIFFS, "/style.css");
    _server.serveStatic("/script.js",   SPIFFS, "/script.js");
    _server.serveStatic("/images",      SPIFFS, "/images");

    setupWebSocket();
    setupRoutes();

    // ElegantOTA Setup (Basic-Auth admin/admin; bei Bedarf ändern)
    ElegantOTA.begin(&_server); // No credentials by default; see docs for auth

    _server.begin();
    Serial.println("✅ Async Webserver gestartet.");
}

void WebServerClass::loop() {
    static unsigned long previousMillis = 0;
    if (millis() - previousMillis >= 1000) {
        previousMillis = millis();
        _counter++;
    }

    // WebSocket Broadcast + ggf. Blinken
    if (millis() - _lastWsBroadcast >= 1000) {
        if (_blinkState) {
            _ledState = !_ledState;
            digitalWrite(_ledPin, _ledState ? HIGH : LOW);
        }
        JsonDocument doc;
        doc["uptime"] = millis() / 1000;
        doc["led"] = _ledState;

        String json;
        serializeJson(doc, json);
        _ws.textAll(json);

        _lastWsBroadcast = millis();
    }

    // Geplanter Neustart?
    if (_pendingRestartAt != 0 && millis() >= _pendingRestartAt) {
        Serial.println("🔄 Neustart wird jetzt ausgeführt...");
        delay(100);
        ESP.restart();
    }
}

void WebServerClass::connectOrStartAP() {
    if (_ssid.isEmpty() || _password.isEmpty()) {
        Serial.println("⚠️  Keine WLAN-Daten gesetzt – starte AP.");
        startAP();
        return;
    }

    Serial.printf("🔌 Verbinde zu %s, %s ...\n", _ssid.c_str(), _password.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid.c_str(), _password.c_str());

    const unsigned long timeoutMs = 12000;
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
        delay(400);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n📡 WLAN verbunden");
        Serial.print("🌐 IP-Adresse: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n⛔ Konnte nicht verbinden – starte AP.");
        startAP();
    }
}

void WebServerClass::startAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(_apIP, _apGW, _apSN);
    WiFi.softAP(_apSsid.c_str(), _apPassword.c_str());
    delay(200); // kurze Stabilisierung
    Serial.printf("📶 AP gestartet: SSID=%s, PASS=%s\n", _apSsid.c_str(), _apPassword.c_str());
    Serial.print("🌐 AP-IP: "); Serial.println(WiFi.softAPIP());
}

void WebServerClass::setupWebSocket() {
    _ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client,
                       AwsEventType type, void *arg, uint8_t *data, size_t len) {

        if (type == WS_EVT_CONNECT) {
            DBG_PRINTLN("WebSocket verbunden");
            client->text("{\"status\":\"verbunden\"}");
            return;
        }
        if (type == WS_EVT_DATA) {
            String msg;
            msg.reserve(len + 1);
            for (size_t i = 0; i < len; i++) msg += (char)data[i];

            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, msg);
            if (err) {
                Serial.println("Ungültiges JSON empfangen");
                return;
            }

            if (doc["blink"].is<bool>()) {
                _blinkState = doc["blink"];
                DBG_PRINTLN(_blinkState ? "Blinken aktiviert" : "Blinken deaktiviert");
            }
            if (doc["led"].is<bool>()) {
                _ledState = doc["led"];
                digitalWrite(_ledPin, _ledState ? HIGH : LOW);
                DBG_PRINTLN(_ledState ? "LED AN" : "LED AUS");
            }
        }
    });
    _server.addHandler(&_ws);
}

void WebServerClass::setupRoutes() {
    auto sendPage = [this](AsyncWebServerRequest *request, 
                        const char* content, 
                        const std::map<String, String> &replacements) {
        String output = html_template;
        output.replace("_BODY_CONTENT_", content);
        for (const auto& pair : replacements) {
            output.replace("%" + pair.first + "%", pair.second);
        }
        request->send(200, "text/html", output);
    };

    // Redirect Root -> /index (bestehende Startseite)
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->redirect("/index");
    });
    //----------------------------------------------------------------------------
    // Startseite erstellt mit html_template und index.html
    //----------------------------------------------------------------------------
    _server.on("/index", HTTP_GET, [this](AsyncWebServerRequest *request) {
        std::map<String, String> values = {
            {"UPTIME", String(millis() / 1000)}
        };
        sendDynamicFile(request, "/index.html", values);
    });
    //----------------------------------------------------------------------------
    // Status-Seite erstellt mit htm_template und status_content aus html_pages.h
    //----------------------------------------------------------------------------
    _server.on("/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        ConfigManager::readString(_eepromText);
        std::map<String, String> replacements = {
            {"EEPROM_TEXT", _eepromText},
            {"SET_COUNTER", "0"},
            {"CUR_COUNTER", String(_counter)}
        };
        sendDynamicPage(request, status_content, replacements);
    });
    // Daten entgegennehmen und in EEPROM speichern
    _server.on("/save_eeprom", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (request->hasParam("data", true)) {
            String receivedData = request->getParam("data", true)->value();
            if (_eepromText != receivedData) {
                _eepromText = receivedData;

                // [EEPROM_WRITE] -> hier später mit deiner EEPROM-Klasse ersetzen
                ConfigManager::writeString(_eepromText);
                Serial.println("EEPROM-Text geändert: " + receivedData);
            }
            request->send(200, "text/plain", "OK");
        } else {
            request->send(400, "text/plain", "Bad Request");
        }
    });
    // Zähler zurücksetzen
    _server.on("/counter_reset", HTTP_GET, [this](AsyncWebServerRequest *request){
        _counter = 0;
        request->send(200, "text/plain", "Ok");
    });
    // Zähler neu setzen
    _server.on("/set_counter", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (request->hasParam("data", true)) {
            String receivedData = request->getParam("data", true)->value();
            _counter = receivedData.toInt();
        }
        request->send(200, "text/plain", "Ok");
    });
    // Counter endpoints
    _server.on("/counter", HTTP_GET, [this](AsyncWebServerRequest *request){
        request->send(200, "text/plain", String(_counter));
    });
    // JSON-Status
    _server.on("/status.json", HTTP_GET, [this](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["uptime_sec"]   = millis() / 1000;
        doc["counter"]      = _counter;
        doc["wifi_mode"]    = currentMode();
        doc["wifi_ssid"]    = (WiFi.getMode() & WIFI_STA) ? WiFi.SSID() : _apSsid;
        doc["ip"]           = currentIP();
        doc["subnet"]       = currentSubnet();
        doc["free_heap"]    = ESP.getFreeHeap();

        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });
    //----------------------------------------------------------------------------
    // WebSocket Demo Seite websocket.html
    //----------------------------------------------------------------------------
    _server.on("/websocket", HTTP_GET, [this](AsyncWebServerRequest *request) {
        sendDynamicFile(request, "/websocket.html", {});
    });
    //----------------------------------------------------------------------------
    // Konfiguration Seite erstellt mit html_template und index.html
    //----------------------------------------------------------------------------
    _server.on("/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        std::map<String, String> replacements = {
            {"CUR_IP",    currentIP()},
            {"CUR_SUB",   currentSubnet()},
            {"STA_SSID",  (WiFi.getMode() & WIFI_STA) ? WiFi.SSID() : _ssid},
            {"STA_PASS",   _password},
            {"IP_ADR",    _locIP.toString()},
            {"SN_MASK",    _locSN.toString()},
            {"AP_SSID",   _apSsid},
            {"AP_PASS",   _apPassword},
            {"AP_IP_ADR", _apIP.toString()},
            {"AP_GW_ADR", _apGW.toString()},
            {"AP_SN_MASK",_apSN.toString()}    
        };
        sendDynamicFile(request, "/config.html", replacements);
    });
    //------------ Speichern der Client (STA) Konfiguration ----------------
    _server.on("/save_sta", HTTP_POST, [this](AsyncWebServerRequest *request) {
        auto getP = [&](const String& name)->String{
            return request->hasParam(name, true) ? request->getParam(name, true)->value() : "";
        };
        String newStaSsid = getP("sta_ssid");
        String newStaPass = getP("sta_pass");
        String newStaIP = getP("sta_ip");
        String newStaSN = getP("sta_sn");
        String staReboot  = getP("sta_reboot"); // "on" wenn Checkbox

        if (newStaSsid.length()) _ssid = newStaSsid;
        if (newStaPass.length()) _password = newStaPass;
        if (newStaIP.length()) _locIP.fromString(newStaIP);
        if (newStaSN.length()) _locSN.fromString(newStaSN);

        saveEEPROMWifiConf(false);
        
        bool doReboot = (staReboot == "on");

        String msg = "Konfiguration gespeichert.";
        if (doReboot) {
            msg += " Neustart in 2 Sekunden...";
            _pendingRestartAt = millis() + 2000;
            request->send(200, "text/plain", msg);
        }
        else         request->redirect("/config");        
    });
    //------------ Speichern der Access Point (AP) Konfiguration ----------------
    _server.on("/save_ap", HTTP_POST, [this](AsyncWebServerRequest *request) {
        auto getP = [&](const String& name)->String{
            return request->hasParam(name, true) ? request->getParam(name, true)->value() : "";
        };
        String newApSsid  = getP("ap_ssid");
        String newApPass  = getP("ap_pass");
        String newApIP = getP("ap_ip");
        String newApGW = getP("ap_gw");
        String newApSN = getP("ap_sn");
        String apReboot   = getP("ap_reboot");

        if (newApSsid.length()) _apSsid = newApSsid;
        if (newApPass.length()) _apPassword = newApPass;
        if (newApIP.length()) _apIP.fromString(newApIP);
        if (newApGW.length()) _apGW.fromString(newApGW);
        if (newApSN.length()) _apSN.fromString(newApSN);

        saveEEPROMWifiConf(true);

        bool doReboot = (apReboot == "on");

        String msg = "Konfiguration AP gespeichert.";
        if (doReboot) {
            msg += " Neustart in 2 Sekunden...";
            _pendingRestartAt = millis() + 2000;
        }
        request->redirect("/config");
    });
    //------------ Beispiel für eine weitere Seite ----------------------------
    // z.B. für eine weitere Seite mit eigener HTML-Datei im SPIFFS
    //----------------------------------------------------------------------------
    _server.on("/test", HTTP_GET, [this](AsyncWebServerRequest *request) {
        File file = SPIFFS.open("/test.html", "r");
        if (!file) {
            request->send(404, "text/plain", "Datei nicht gefunden");
            return;
        }
        String page = file.readString();
        file.close();
        request->send(200, "text/html", page);
    });
    // 404 für alles andere
    _server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Seite nicht gefunden");
    });
}

void WebServerClass::sendDynamicPage(AsyncWebServerRequest *request,
                                     const String &content,
                                     const std::map<String, String> &replacements)
{
    String output = html_template;
    output.replace("_BODY_CONTENT_", content);

    for (auto const &pair : replacements) {
        output.replace("%" + pair.first + "%", pair.second);
    }
    
    request->send(200, "text/html", output);
}

void WebServerClass::sendDynamicFile(AsyncWebServerRequest *request,
                                     const char* path,
                                     const std::map<String, String> &replacements)
{
    File file = SPIFFS.open(path, "r");
    if (!file) {
        request->send(404, "text/plain", "Datei nicht gefunden");
        return;
    }
    String content = file.readString();
    file.close();

    sendDynamicPage(request, content, replacements);
}

// Nur Marker – du setzt hier später deine EEPROM-Klasse ein
void WebServerClass::loadEEPROMWifiConf(bool ap) {
//    Serial.println("[EEPROM_READ] Platzhalter – hier später Implementierung einfügen.");
    WifiConf wifiConf;
    ConfigManager::readWifiConf(wifiConf, ap);
    if (!ap) {
        if (String(wifiConf.ssid).length() > 0) _ssid = String(wifiConf.ssid);
        else _ssid = "";
        if (String(wifiConf.password).length() > 0)_password = String(wifiConf.password);
        else _password = "";
        _locIP = wifiConf.ip;
        _locSN = wifiConf.sn;
    } else {
        if (String(wifiConf.ssid).length() > 0) _apSsid = String(wifiConf.ssid);
        else _apSsid = "ESP32-Setup";
        if (String(wifiConf.password).length() > 0) _apPassword = String(wifiConf.password);
        else _apPassword = "admin";
        _apIP = wifiConf.ip;    
        _apGW = wifiConf.gw;    
        _apSN = wifiConf.sn;        
    }
}
void WebServerClass::saveEEPROMWifiConf(bool ap) {
    // Serial.println("[EEPROM_WRITE] Platzhalter – hier später Implementierung einfügen.");
    WifiConf wifiConf{};
    if(!ap) {
        strncpy(wifiConf.ssid, _ssid.c_str(), sizeof(wifiConf.ssid));
        strncpy(wifiConf.password, _password.c_str(), sizeof(wifiConf.password));
        wifiConf.ip = _locIP;
        wifiConf.sn = _locSN;   
    } else {
        strncpy(wifiConf.ssid, _apSsid.c_str(), sizeof(wifiConf.ssid));
        strncpy(wifiConf.password, _apPassword.c_str(), sizeof(wifiConf.password)); 
        wifiConf.ip = _apIP;
        wifiConf.gw = _apGW;
        wifiConf.sn = _apSN;       
    }
    ConfigManager::writeWifiConf(wifiConf, ap);
}
void WebServerClass::loadEEPROMText(String &text) {
    ConfigManager::readString(text);
}
void WebServerClass::saveEEPROMText(const String &text) {
    ConfigManager::writeString(text);
}

// Helfer
String WebServerClass::currentIP() {
    if (WiFi.getMode() & WIFI_STA && WiFi.status() == WL_CONNECTED) {
        return WiFi.localIP().toString();
    }
    return WiFi.softAPIP().toString();
}
String WebServerClass::currentSubnet() {
    if (WiFi.getMode() & WIFI_STA && WiFi.status() == WL_CONNECTED) {
        return WiFi.subnetMask().toString();
    }
    return _apSN.toString();
}
String WebServerClass::currentMode() {
    if (WiFi.getMode() & WIFI_STA && WiFi.status() == WL_CONNECTED) return "STA";
    if (WiFi.getMode() & WIFI_AP) return "AP";
    return "UNKNOWN";
}
