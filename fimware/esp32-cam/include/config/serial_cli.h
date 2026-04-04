#ifndef SERIAL_CLI_H
#define SERIAL_CLI_H

#include <Arduino.h>

#include "config/config_store.h"

namespace serial_cli {

void printHelp();
void handle(AppConfig &cfg);

}  // namespace serial_cli

#endif
