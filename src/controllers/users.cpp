#include "users.h"

#include "src/managers/adminmanager.h"
#include <QSqlRecord>

bool UserController::createUser(QVariantMap& params, QString* error) {
  AdminManager am;
  if (am.exists(params["username"].toString())) {
    if (error) *error = "Username already exists";
    return false;
  }

  auto opt = am.create(params);
  if (!opt.has_value()) {
    if (error) *error = am.errorString();
    return false;
  }
  params["id"] = opt->value("id").toInt();
  return true;
}

bool UserController::updateUser(int userId, const QVariantMap& params,
                                QString* error) {
  AdminManager am;
  if (am.update(userId, params)) return true;

  if (error) *error = am.errorString();
  return false;
}

UserData UserController::getUserData(int id) const {
  UserData ud{.id = -1, .role_id = -1};
  QSqlQuery q(BaseManager::connection);
  q.prepare("SELECT * FROM admins WHERE id = :id");
  q.bindValue(":id", id);
  if (q.exec() && q.next()) 
  {
    ud.id =  id;
    ud.role_id = q.value("role_id").toInt();
    ud.email = q.value("email").toString();
    ud.nomor_telp = q.value("nomor_telp").toString();
    ud.nama_lengkap = q.value("nama_lengkap").toString();
  }
  return ud;
}

QSqlRecord UserController::getUserRecord(int id) const { 
  QSqlRecord rec;
  QSqlQuery q(BaseManager::connection);
  q.prepare("SELECT * FROM admins WHERE id = :id");
  q.bindValue(":id", id);
  if (q.exec() && q.next()) return q.record();
  return rec;
}
