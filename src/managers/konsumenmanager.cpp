#include "konsumenmanager.h"
#include <QDateTime>

void KonsumenManager::beforeCreate(QVariantMap &vm) {
  vm["created_at"] = QDateTime::currentDateTimeUtc();
  vm["updated_at"] = QDateTime::currentDateTimeUtc();
}

void KonsumenManager::beforeUpdate(int id, QVariantMap &vm) {
  vm["updated_at"] = QDateTime::currentDateTimeUtc();
}

int KonsumenManager::getPriceLevelById(int idk) const {
  auto q = BaseManager::baseQuery();
  q.prepare("SELECT price_level_id FROM konsumen WHERE id = :id");
  q.bindValue(":id", idk);
  if (q.exec() && q.next()) {
    return q.value(0).toInt();
  }
  return -1;
}