#ifndef CLIMANAGER_H
#define CLIMANAGER_H

#include <Arduino.h>
#include "ConfigManager.h"

class CliManager {
public:
    static void begin();
    static void handle();
private:
    static void showMenu();
    static void processCommand(char cmd);
};
#endif
