#include "adminmanager.h"
#include "src/utils/authmanager.h"
#include <QSqlQuery>
#include <QDateTime>


bool AdminManager::exists(const QString& username) {
  QSqlQuery query(BaseManager::connection);
  query.prepare("SELECT COUNT(*) AS total from admins WHERE username = :username");
  query.bindValue(":username", username);
  if(query.exec() && query.next()) {
    return query.value("total").toInt() > 0;
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
  param["created_at"] = QDateTime::currentDateTimeUtc();
  param["updated_at"] = QDateTime::currentDateTimeUtc();
}

void AdminManager::beforeUpdate(QVariantMap &param) {
  param.remove("created_at");
  param["updated_at"] = QDateTime::currentDateTimeUtc();
}