#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#include "config/config_store.h"
#include "system/status.h"

namespace wifi_service {

app::Status connect(const AppConfig &cfg);
app::Status ensureConnected(const AppConfig &cfg);

}  // namespace wifi_service

#endif
