#include "konsumenmanager.h"

QSqlRecord KonsumenManager::create(const QVariantMap& map) {
  if(!map.contains("nama_lengkap")) {
    return QSqlRecord();
  }
  return BaseManager::create(map);
}