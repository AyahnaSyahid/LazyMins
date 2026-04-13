#include "konsumenmanager.h"
#include <QDateTime>

bool KonsumenManager::beforeCreate(QVariantMap &vm) {
  vm["created_at"] = QDateTime::currentDateTimeUtc();
  vm["updated_at"] = QDateTime::currentDateTimeUtc();
  return true;
}

bool KonsumenManager::beforeUpdate(int id, QVariantMap &vm) {
  vm["updated_at"] = QDateTime::currentDateTimeUtc();
  return true;
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

QVariantMap KonsumenManager::validateParams(const QVariantMap& params) {
  auto newParams = BaseManager::validateParams(params);
  QString customer_code = newParams["customer_code"].toString();
  QString catatan = newParams["catatan"].toString();
  if(customer_code.simplified().isEmpty()) newParams["customer_code"] = QVariant(QMetaType::fromType<QString>());
  if(catatan.simplified().isEmpty())       newParams["catatan"]       = QVariant(QMetaType::fromType<QString>());
  return newParams;
}