#include "WebServerClass.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>
#include "html_template.h"
#include "html_pages.h"

WebServerClass::WebServerClass()
: _server(80), _ws("/ws") {}

void WebServerClass::setCredentials(const String& ssid, const String& password) {
    _ssid = ssid;
    _password = password;
}

void WebServerClass::begin() {
    Serial.begin(115200);
    pinMode(_ledPin, OUTPUT);
    digitalWrite(_ledPin, HIGH);

    if (!SPIFFS.begin(true)) {
        Serial.println("Fehler beim Mounten von SPIFFS");
    }

    connectOrStartAP();

    // Statische Assets
    _server.serveStatic("/favicon.ico", SPIFFS, "/favicon.ico");
    _server.serveStatic("/style.css",   SPIFFS, "/style.css");
    _server.serveStatic("/script.js",   SPIFFS, "/script.js");
    _server.serveStatic("/images",      SPIFFS, "/images");

    loadEEPROMText();

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

    Serial.printf("🔌 Verbinde zu %s ...\n", _ssid.c_str());
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
            Serial.println("WebSocket verbunden");
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
                Serial.println(_blinkState ? "Blinken aktiviert" : "Blinken deaktiviert");
            }
            if (doc["led"].is<bool>()) {
                _ledState = doc["led"];
                digitalWrite(_ledPin, _ledState ? HIGH : LOW);
                Serial.println(_ledState ? "LED AN" : "LED AUS");
            }
        }
    });

    _server.addHandler(&_ws);
}

void WebServerClass::setupRoutes() {
    auto sendPage = [this](AsyncWebServerRequest *request, const char* content) {
        String output = html_template;
        output.replace("_BODY_CONTENT_", content);
        output.replace("%COUNTER_VALUE%", String(_counter));
        output.replace("%EEPROM_TEXT%", _eepromText);
        output.replace("%SET_COUNTER%", String("1.12f"));
        output.replace("%CUR_COUNTER%", String("0"));
        request->send(200, "text/html", output);
    };

    // Redirect Root -> /index (bestehende Startseite)
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->redirect("/index");
    });

    // Alte Startseite über SPIFFS template.html + /index.html (falls vorhanden)
    _server.on("/index", HTTP_GET, [this](AsyncWebServerRequest *request) {
        std::map<String, String> values = {
            {"UPTIME", String(millis() / 1000)}
        };
        sendDynamicFile(request, "/index.html", values);
    });

    // Submenu Demo
    _server.on("/submenu01", HTTP_GET, [=](AsyncWebServerRequest *request){
        sendPage(request, submenu01_content);
    });

    // Counter endpoints
    _server.on("/counter", HTTP_GET, [this](AsyncWebServerRequest *request){
        request->send(200, "text/plain", String(_counter));
    });
    _server.on("/counterReset", HTTP_GET, [=](AsyncWebServerRequest *request){
        _counter = 0;
        request->send(200, "text/plain", "Ok");
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

    // Status-Seite (HTML, wie zuvor)
    _server.on("/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        std::map<String, String> dynValues = {
            {"%SET_COUNTER%", "22.5"},
            {"%CUR_COUNTER%", String(_counter)}
        };
        sendDynamicPage(request, status_content, dynValues);
    });

    // WebSocket Demo Seite (umbenannt: websocket.html)
    _server.on("/websocket", HTTP_GET, [this](AsyncWebServerRequest *request) {
        sendDynamicFile(request, "/websocket.html", {});
    });

    // Konfiguration (Datei-basiert, wie index)
    _server.on("/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        std::map<String, String> values = {
            {"STA_SSID",  (WiFi.getMode() & WIFI_STA) ? WiFi.SSID() : _ssid},
            {"AP_SSID",   _apSsid},
            {"CUR_IP",    currentIP()},
            {"CUR_SUB",   currentSubnet()}
        };
        sendDynamicFile(request, "/config.html", values);
    });

    // Daten entgegennehmen (ohne EEPROM, nur merken/anzeigen)
    _server.on("/save_config", HTTP_POST, [this](AsyncWebServerRequest *request) {
        auto getP = [&](const String& name)->String{
            return request->hasParam(name, true) ? request->getParam(name, true)->value() : "";
        };

        String newStaSsid = getP("sta_ssid");
        String newStaPass = getP("sta_pass");
        String staReboot  = getP("sta_reboot"); // "on" wenn Checkbox

        String newApSsid  = getP("ap_ssid");
        String newApPass  = getP("ap_pass");
        String apReboot   = getP("ap_reboot");

        if (newStaSsid.length()) _ssid = newStaSsid;
        if (newStaPass.length()) _password = newStaPass;

        if (newApSsid.length()) _apSsid = newApSsid;
        if (newApPass.length()) _apPassword = newApPass;

        Serial.printf("[CONFIG] STA: ssid=%s pass=%s | AP: ssid=%s pass=%s\n",
                      _ssid.c_str(), _password.c_str(), _apSsid.c_str(), _apPassword.c_str());
        Serial.println("[EEPROM_WRITE] (Platzhalter) – Werte würden hier gespeichert werden.");

        bool doReboot = (staReboot == "on") || (apReboot == "on");

        String msg = "Konfiguration gespeichert.";
        if (doReboot) {
            msg += " Neustart in 2 Sekunden...";
            _pendingRestartAt = millis() + 2000;
        }
        request->send(200, "text/plain", msg);
    });

    // Uptime
    _server.on("/uptime", HTTP_GET, [this](AsyncWebServerRequest *request){
        request->send(200, "text/plain", String(millis() / 1000));
    });

    // SPIFFS-Root optional ausliefern (falls du zusätzlich Files direkt brauchst)
    // _server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
}

void WebServerClass::sendDynamicPage(AsyncWebServerRequest *request,
                                     const String &baseContent,
                                     const std::map<String, String> &replacements)
{
    String output = html_template;
    String content = baseContent;

    for (auto const &pair : replacements) {
        content.replace(pair.first, pair.second);
    }
    output.replace("_BODY_CONTENT_", content);
    request->send(200, "text/html", output);
}

void WebServerClass::sendDynamicFile(AsyncWebServerRequest *request,
                                     const char* path,
                                     const std::map<String, String> &values)
{
    File file = SPIFFS.open(path, "r");
    if (!file) {
        request->send(404, "text/plain", "Datei nicht gefunden");
        return;
    }
    String page = file.readString();
    file.close();

    for (auto &pair : values) {
        page.replace("%" + pair.first + "%", pair.second);
    }

    File tmpl = SPIFFS.open("/template.html", "r");
    if (!tmpl) {
        request->send(500, "text/plain", "Template-Datei nicht gefunden");
        return;
    }
    String base_template = tmpl.readString();
    tmpl.close();

    base_template.replace("_BODY_CONTENT_", page);
    request->send(200, "text/html", base_template);
}

// Nur Marker – du setzt hier später deine EEPROM-Klasse ein
void WebServerClass::loadEEPROMText() {
    Serial.println("[EEPROM_READ] Platzhalter – hier später Implementierung einfügen.");
}
void WebServerClass::saveEEPROMText(const String& data) {
    Serial.println("[EEPROM_WRITE] Platzhalter – hier später Implementierung einfügen.");
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
