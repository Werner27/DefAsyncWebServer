# ESP32 WebServer Basisprojekt

Dieses Projekt stellt eine wiederverwendbare WebServer-Klasse für den ESP32 bereit, die als Grundlage für zukünftige IoT- und Webinterface-Projekte dient.

## ✨ Features

- Automatischer Fallback auf Access Point bei WLAN-Verbindungsfehler
- Dynamische HTML-Seiten mit Template-System
- Konfigurationsseite für STA/AP-Modus
- JSON-Statusseite mit Live-Daten
- WebSocket-Demo zur LED-Steuerung
- OTA-Update über ElegantOTA
- Modularer Aufbau für einfache Erweiterung

## 📁 Projektstruktur

- `WebServerClass.*` – zentrale Serverlogik
- `html_pages.h` – HTML-Inhalte als C++-Strings
- `html_template.h` – Template für dynamische Seiten
- `SPIFFS` – enthält `index.html`, `config.html`, `websocket.html`, `style.css`, `script.js`

## 🚀 Installation

1. ESP32 mit PlatformIO oder Arduino IDE einrichten
2. Projekt klonen:
   ```bash
   git clone https://github.com/werner27/DefAsyncWebServer.git
3. SPIFFS-Dateien hochladen
4. Kompilieren und flashen
