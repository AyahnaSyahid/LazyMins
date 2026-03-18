#include "konsumenmanager.h"
#include <QDateTime>

void KonsumenManager::beforeCreate(QVariantMap &vm) {
  vm["created_at"] = QDateTime::currentDateTimeUtc();
  vm["updated_at"] = QDateTime::currentDateTimeUtc();
}

void KonsumenManager::beforeUpdate(int id, QVariantMap &vm) {
  vm["updated_at"] = QDateTime::currentDateTimeUtc();
}