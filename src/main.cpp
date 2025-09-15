#include <Arduino.h>
#include "WebServerClass.h"
#include <SPIFFS.h>


// Erstelle ein Objekt deiner Webserver-Klasse
WebServerClass myWebServer;

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void setup() {
  Serial.begin(115200);
  myWebServer.setCredentials("YOUR_SSID", "YOUR_PASSWORD"); // Hier deine WLAN-Zugangsdaten eintragen
  myWebServer.begin();
}

void loop() {
  myWebServer.loop();
}