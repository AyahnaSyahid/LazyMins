#include "konsumenmanager.h"

std::optional<QSqlRecord> KonsumenManager::create(const QVariantMap& map) {
  if(!map.contains("nama_lengkap")) {
    return std::nullopt;
  }
  return BaseManager::create(map);
}