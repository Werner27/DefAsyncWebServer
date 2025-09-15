#include "ConfigManager.h"

void ConfigManager::begin() {
  EEPROM.begin(EEPROM_SIZE);
  DBG_PRINT("EEPROM length: ");
  DBG_PRINTLN(EEPROM.length());
}

void ConfigManager::readWifiConf(WifiConf &wifiConf) {
  EEPROM.get(EE_ADD_WIFI, wifiConf);
  wifiConf.ssid[MAX_SSID-1] = '\0';
  wifiConf.password[MAX_PASSWORD-1] = '\0';

  DBG_PRINT("SSID: "); DBG_PRINTLN(wifiConf.ssid);
  DBG_PRINT("Password: "); DBG_PRINTLN(wifiConf.password);
}

void ConfigManager::writeWifiConf(const WifiConf &wifiConf) {
  DBG_PRINT("SSID: "); DBG_PRINTLN(wifiConf.ssid);
  DBG_PRINT("Password: "); DBG_PRINTLN(wifiConf.password);

  EEPROM.put(EE_ADD_WIFI, wifiConf);
  EEPROM.commit();
}

void ConfigManager::readByte(byte &b) {
  b = EEPROM.read(EE_ADD_BYTE);
  DBG_PRINT("Byte: "); DBG_PRINTLN(b);
}

void ConfigManager::writeByte(const byte &b) {
  DBG_PRINT("Byte: "); DBG_PRINTLN(b);
  EEPROM.write(EE_ADD_BYTE, b);
  EEPROM.commit();
}

void ConfigManager::readFloat(float &f) {
  EEPROM.get(EE_ADD_FLOAT, f);
  DBG_PRINT("Float: "); DBG_PRINTLN(f, 3);
}

void ConfigManager::writeFloat(const float &f) {
  DBG_PRINT("Float: "); DBG_PRINTLN(f, 3);
  EEPROM.put(EE_ADD_FLOAT, f);
  EEPROM.commit();
}

void ConfigManager::readMyTestObject(MyTestObject &obj) {
  EEPROM.get(EE_ADD_MY_TEST_OBJECT, obj);
  DBG_PRINT("byte_field: "); DBG_PRINTLN(obj.byte_field);
  DBG_PRINT("long_field: "); DBG_PRINTLN(obj.long_field);
  DBG_PRINT("float_field: "); DBG_PRINTLN(obj.float_field);
  DBG_PRINT("name: "); DBG_PRINTLN(obj.name);
}

void ConfigManager::writeMyTestObject(const MyTestObject &obj) {
  DBG_PRINT("byte_field: "); DBG_PRINTLN(obj.byte_field);
  DBG_PRINT("long_field: "); DBG_PRINTLN(obj.long_field);
  DBG_PRINT("float_field: "); DBG_PRINTLN(obj.float_field);
  DBG_PRINT("name: "); DBG_PRINTLN(obj.name);

  EEPROM.put(EE_ADD_MY_TEST_OBJECT, obj);
  EEPROM.commit();
}
