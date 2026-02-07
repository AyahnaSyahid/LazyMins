#include "adminmanager.h"
#include "src/utils/authmanager.h"
#include <QSqlDatabase>
#include <QSqlQuery>

QSqlRecord AdminManager::create(CreateAdminParams &cap)
{
  QSqlDatabase _db = QSqlDatabase::database("LMAdmins_db");
  if (_db.isValid()) {
    qDebug() << "db is valid";
    if(!_db.isOpen()) {
      if(!_db.open()) {
        return QSqlRecord();
      }
    }
    qDebug() << "db is open";
    auto &am = AuthManager::instance();
    QString salt = am.generateSalt();
    QString hash = am.generateHash(cap.literal_password, salt);
    QSqlQuery q(_db);
    q.prepare(R"-(
      INSERT INTO admins ( username, 
                           password_hash, 
                           salt, 
                           nama_lengkap, 
                           email, 
                           nomor_telepon )
      VALUES ( :username, 
               :password_hash, 
               :salt, 
               :nama_lengkap, 
               :email, 
               :nomor_telepon ))-");

    q.bindValue(":username", cap.username);
    q.bindValue(":password_hash", hash);
    q.bindValue(":salt", salt);
    q.bindValue(":nama_lengkap", cap.nama_lengkap);
    q.bindValue(":email", cap.email);
    q.bindValue(":nomor_telepon", cap.nomor_telepon);
    
    if (q.exec()) {
      int id = q.lastInsertId().toInt();
      QSqlQuery s(_db);
      s.prepare("SELECT * FROM admins WHERE id = :id");
      s.bindValue(":id", id);
      if(s.exec() && s.next()) {
        return s.record();
      }
    }
    _db.rollback();
  }
  return QSqlRecord();
}

QSqlRecord AdminManager::getById(int id)
{
  auto db = QSqlDatabase::database("LMAdmins_db");
  QSqlQuery q(db);
  q.prepare("SELECT * FROM admins WHERE id=:id");
  q.bindValue(":id", id);
  if(q.exec() && q.next()) {
    return q.record();
  }
  return QSqlRecord();
}

QSqlRecord AdminManager::getByUsername(const QString& username)
{
  auto db = QSqlDatabase::database("LMAdmins_db");
  QSqlQuery q(db);
  q.prepare("SELECT * FROM admins WHERE username=:username");
  q.bindValue(":username", username);
  if(q.exec() && q.next()) {
    return q.record();
  }
  return QSqlRecord();
}

bool AdminManager::remove(int adminId)
{
  auto db = QSqlDatabase::database("LMAdmins_db");
  QSqlQuery q(db);
  q.prepare("DELETE FROM admins WHERE id=:id");
  q.bindValue(":id", adminId);
  return q.exec();
}

bool AdminManager::setRole(int adminId, int role_id)
{
  auto db = QSqlDatabase::database("LMAdmins_db");
  QSqlQuery q(db);
  q.prepare("UPDATE admins SET role_id = :new_role WHERE id = :id");
  q.bindValue(":new_role", role_id);
  q.bindValue(":id", adminId);
  return q.exec();
}

bool AdminManager::setStatus(int adminId, int state)
{
  auto db = QSqlDatabase::database("LMAdmins_db");
  QSqlQuery q(db);
  q.prepare("UPDATE admins SET is_active = :new_state WHERE id = :id");
  q.bindValue(":new_state", state);
  q.bindValue(":id", adminId);
  return q.exec();
}

void AdminManager::updateLastLogin(int adminId)
{
  auto db = QSqlDatabase::database("LMAdmins_db");
  QSqlQuery q(db);
  q.prepare("UPDATE admins SET last_login = CURRENT_TIMESTAMP WHERE id = :id");
  q.bindValue(":id", adminId);
  q.exec();
}
  