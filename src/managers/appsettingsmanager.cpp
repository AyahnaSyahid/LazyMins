#include "appsettingsmanager.h"
#include "src/managers/basemanager.h"
#include <QSqlQuery>
#include <QSqlRecord>


QVariantMap AppSettingsManager::getSettings(const QString& name) const 
{
  auto q = BaseManager::baseQuery();
  q.prepare("SELECT * FROM app_settings WHERE setting_key = :name");
  q.bindValue(":name", name);
  if (q.exec() && q.next()) {
    auto record = q.record();
    QVariantMap map;
    for (int i = 0; i < record.count(); ++i) {
        // fieldName(i) mengambil nama kolom, value(i) mengambil datanya
        map.insert(record.fieldName(i), record.value(i));
    }
    return map;
  }
  return QVariantMap {};
};