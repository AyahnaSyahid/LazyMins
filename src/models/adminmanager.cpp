#include "adminmanager.h"
#include "src/utils/authmanager.h"
#include <QSqlQuery>

QSqlRecord AdminManager::create(const QVariantMap &map) {
  QSqlRecord record;
  QStringList required_keys = { "username", "literal_password", "nama_lengkap" };
  bool canPass = true;
  for (const QString& key : required_keys) {
    if (!map.contains(key)) {
      qDebug() << "Missing required field :" << key;
      return record;
    }
  }
  
  QVariantMap param(map);
  beforeCreate(param);
  
  return BaseManager::create(param);
}

bool AdminManager::exists(const QString& username) {
  QSqlQuery query(BaseManager::connection);
  query.prepare("SELECT COUNT(*) AS total from admins WHERE username = :username");
  query.bindValue(":username", username);
  if(query.exec() && query.next()) {
    return query.value(0).toInt() > 0;
  }
  return false;
}

void AdminManager::beforeCreate(QVariantMap &param) {
  auto &am = AuthManager::instance();
  QString salt = am.generateSalt();
  QString hash = am.generateHash(param["literal_password"].toString(), salt);
  
  param.remove("literal_password");
  param["salt"] = salt;
  param["password_hash"] = hash;
}