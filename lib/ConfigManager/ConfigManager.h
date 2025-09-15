#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <Arduino.h>
#include <EEPROM.h>

// --- Debug-Steuerung ---
// Über platformio.ini:
// build_flags = -DSERIAL_VERBOSE
#ifdef SERIAL_VERBOSE
  #define DBG_PRINT(...)    Serial.print(__VA_ARGS__)
  #define DBG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
  #define DBG_PRINT(...)
  #define DBG_PRINTLN(...)
#endif

// --- Konfiguration ---
constexpr size_t EEPROM_SIZE = 512;
constexpr size_t MAX_SSID = 21;
constexpr size_t MAX_PASSWORD = 21;

// --- EEPROM-Adressen ---
struct WifiConf {
  char ssid[MAX_SSID];
  char password[MAX_PASSWORD];
};

struct MyTestObject {
  byte  byte_field;
  long  long_field;
  float float_field;
  char  name[10];
};

constexpr int EE_ADD_WIFI           = 0;
constexpr int EE_ADD_BYTE           = EE_ADD_WIFI + sizeof(WifiConf);
constexpr int EE_ADD_FLOAT          = EE_ADD_BYTE + sizeof(byte);
constexpr int EE_ADD_MY_TEST_OBJECT = EE_ADD_FLOAT + sizeof(float);

class ConfigManager {
public:
  ConfigManager() = delete; // keine Instanz nötig
  static void begin();
  static void readWifiConf(WifiConf &wifiConf);
  static void writeWifiConf(const WifiConf &wifiConf);
  static void readByte(byte &b);
  static void writeByte(const byte &b);
  static void readFloat(float &f);
  static void writeFloat(const float &f);
  static void readMyTestObject(MyTestObject &obj);
  static void writeMyTestObject(const MyTestObject &obj);
};

#endif
