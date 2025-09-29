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
  #define DBG_PRINTF(...)   Serial.printf(__VA_ARGS__)
#else
  #define DBG_PRINT(...)
  #define DBG_PRINTLN(...)
  #define DBG_PRINTF(...) 
#endif

// --- Konfiguration ---
constexpr size_t EEPROM_SIZE = 512;
constexpr size_t MAX_SSID = 21;
constexpr size_t MAX_PASSWORD = 21;
constexpr size_t MAX_IP = 17;
constexpr size_t MAX_SN = 17;
constexpr size_t MAX_TEXT = 51;

struct WifiConfChecked {
  String ssid;
  String password;
  IPAddress ip; 
  IPAddress sn;
  IPAddress gw; // nur für AP
};

// --- EEPROM-Adressen ---
struct WifiConf {
  char ssid[MAX_SSID];
  char password[MAX_PASSWORD];
  IPAddress ip; // als 4 Bytes
  IPAddress gw; // nur für AP
  IPAddress sn; // Subnet Mask als 4 Bytes
};

struct MyTestObject {
  byte  byte_field;
  long  long_field;
  float float_field;
  char  name[12];
};

constexpr int EE_ADD_WIFI           = 0;
constexpr int EE_ADD_WIFI_AP        = EE_ADD_WIFI + sizeof(WifiConf);
constexpr int EE_ADD_BYTE           = EE_ADD_WIFI_AP + sizeof(WifiConf);
constexpr int EE_ADD_FLOAT          = EE_ADD_BYTE + sizeof(byte);
constexpr int EE_ADD_TEXT           = EE_ADD_FLOAT + MAX_TEXT;
constexpr int EE_ADD_MY_TEST_OBJECT = EE_ADD_TEXT + sizeof(float);

class ConfigManager {
public:
  ConfigManager() = delete; // keine Instanz nötig
  static void begin();
  static void ip_byte_to_array(const IPAddress &ip, byte bytes[4]);
  static IPAddress ip_from_bytes(const byte bytes[4]);
  static IPAddress CheckIP(const IPAddress &ip, const IPAddress defaultIP = IPAddress(0,0,0,0));
  static IPAddress CheckSN(const IPAddress &sn);
  // ap = true: Access Point, false: Client (Station) 
  static void readWifiConf (WifiConf &wifiConf, bool ap=false);
  static void writeWifiConf(const WifiConf &wifiConf, bool ap=false);
  static void readByte(byte &b);
  static void writeByte(const byte &b);
  static void readFloat(float &f);
  static void writeFloat(const float &f);
  static void readString(String &text);
  static void writeString(const String &text);
  static void readMyTestObject(MyTestObject &obj);
  static void writeMyTestObject(const MyTestObject &obj);
};

#endif
