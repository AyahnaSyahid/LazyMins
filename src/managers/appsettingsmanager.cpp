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

bool AppSettingsManager::saveSettings(const QString& name, const QVariantMap& map) 
{
  resetErrorString();
  auto q = BaseManager::baseQuery();
  q.prepare("SELECT 1 FROM app_settings WHERE setting_key = :key");
  q.bindValue(":key", name);
  if(q.exec() && q.next()) {
    q.prepare("UPDATE app_settings SET "
              "(setting_value, data_type, description, is_public, updated_by, updated_at) = "
              "(:val, :dtype, :desc, :pub, :upby, :upat) WHERE setting_key = :key");
    q.bindValue(":key",   name);
    q.bindValue(":val",   map["setting_value"]);
    q.bindValue(":dtype", map["data_type"]);
    q.bindValue(":desc",  map["description"]);
    q.bindValue(":pub",   map["is_public"]);
    q.bindValue(":upby",  map["updated_by"]);
    q.bindValue(":upat",  QDateTime::currentDateTimeUtc());
    if(!q.exec()) {
      setErrorString(q.lastError().text());
      return false;
    }
    if(q.numRowsAffected() != 1) {
      setErrorString("Jumlah row yang terpengaruh != 1");
      return false;
    }
    return true;
  }
  q.prepare("INSERT INTO app_settings "
            "(setting_key, setting_value, data_type, description, is_public, updated_by, updated_at) "
            "VALUES (:key, :val, :dtype, :desc, :pub, :upby, :upat);");
  q.bindValue(":key",   name);
  q.bindValue(":val",   map["setting_value"]);
  q.bindValue(":dtype", map.value("data_type", "string"));
  q.bindValue(":desc",  map.value("description", "N/A"));
  q.bindValue(":pub",   map.value("is_public","0"));
  q.bindValue(":upby",  map.value("updated_by", "1"));
  q.bindValue(":upat",  QDateTime::currentDateTimeUtc());
  if(!q.exec()) {  
    setErrorString(q.lastError().text());
    return false;
  }
  return true;
}