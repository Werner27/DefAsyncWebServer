#ifndef WEBSERVERCLASS_H
#define WEBSERVERCLASS_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <map>

class WebServerClass {
public:
    WebServerClass();

    void begin();
    void loop();

    void setCredentials(const String& ssid, const String& password);
    void loadEEPROMText(String &text);
    void saveEEPROMText(const String &text);
    void loadEEPROMWifiConf(bool ap = false);
    void saveEEPROMWifiConf(bool ap = false);

private:
    // Web
    AsyncWebServer _server;
    AsyncWebSocket _ws;

    // Standalone (STA) (Client) Credentials
    String _ssid;
    String _password;
    IPAddress  _locIP;
    IPAddress  _locSN;

    // AP Settings
    String _apSsid = "ESP32-Setup";
    String _apPassword = "admin";
    IPAddress _apIP  = IPAddress(192, 168, 10, 1);
    IPAddress _apGW  = IPAddress(192, 168, 10, 1);
    IPAddress _apSN  = IPAddress(255, 255, 255, 0);

    // WebSocket Demo
    int _ledPin = 2;
    bool _ledState = false;
    bool _blinkState = false;
    unsigned long _lastWsBroadcast = 0;

    // Status/sonstiges
    int _counter = 0;
    String _eepromText = "";
    unsigned long _pendingRestartAt = 0; // 0 = kein Neustart geplant

private:
    void connectOrStartAP();
    void startAP();

    void setupRoutes();
    void setupWebSocket();

    void sendDynamicPage(AsyncWebServerRequest *request,
                         const String &baseContent,
                         const std::map<String, String> &replacements = {});
    void sendDynamicFile(AsyncWebServerRequest *request,
                         const char* path,
                         const std::map<String, String> &replacements = {});

    // Helfer für Anzeige auf Config-Seite
    String currentIP();
    String currentSubnet();
    String currentMode(); // "STA" oder "AP"
};

#endif
