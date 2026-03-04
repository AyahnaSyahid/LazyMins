#include "adminmanager.h"
#include "src/utils/authmanager.h"
#include "src/managers/rolesmanager.h"
#include <QSqlQuery>
#include <QDateTime>

bool AdminManager::exists(const QString &username)
{
  auto found = getWhere("username = :uname", {{"uname", username}}, "", 1).count() > 0;
  return found;
}

void AdminManager::beforeCreate(QVariantMap &param)
{
  auto &am = AuthManager::instance();
  QString salt = am.generateSalt();
  QString hash = am.generateHash(param["literal_password"].toString(), salt);

  param.remove("literal_password");
  param["salt"] = salt;
  param["password_hash"] = hash;
  param["created_at"] = QDateTime::currentDateTimeUtc();
  param["updated_at"] = QDateTime::currentDateTimeUtc();
}

void AdminManager::beforeUpdate(int id, QVariantMap &param)
{
  param["updated_at"] = QDateTime::currentDateTimeUtc();
}

bool AdminManager::changeLoginInfo(const QString &oldname, const QString &newName, const QString &newPass)
{
  resetErrorString();
  connection.transaction();
  auto q = baseQuery();
  q.prepare("SELECT id FROM admins WHERE username = :old");
  q.bindValue(":old", oldname);
  if (q.exec() && q.next())
  {
    int id = q.value("id").toInt();
    auto &am = AuthManager::instance();
    auto newSalt = am.generateSalt();
    auto newHash = am.generateHash(newPass, newSalt);
    auto qq = baseQuery();
    qq.prepare("UPDATE admins SET (username, salt, password_hash, updated_at) = "
               "(:newname, :salt, :phash, :ctu) WHERE id = :id");
    qq.bindValue(":newname", newName);
    qq.bindValue(":salt", newSalt);
    qq.bindValue(":phash", newHash);
    qq.bindValue(":ctu", QDateTime::currentDateTimeUtc());
    qq.bindValue(":id", id);

    if (qq.exec())
    {
      if (connection.commit())
      {
        return true;
      }
      else
      {
        setErrorString("Unable to commit : " + connection.lastError().text());
        return false;
      }
      setErrorString("execution failed : " + qq.lastError().text());
      return false;
    }
  }
  setErrorString("Error tidak diketahui");
  connection.rollback();
  return false;
}

bool AdminManager::userHasRole(int userId, const QString &roleName) const
{
  RolesManager rm;
  auto roleIdOpt = rm.roleId(roleName);
  if (!roleIdOpt)  {
    return false;
  }
  auto userOpt = BaseManager::getById(userId);
  if (!userOpt) {
    return false;
  }
  auto userRecord = *userOpt;
  return userRecord.value("role_id").toInt() == *roleIdOpt;
}

bool AdminManager::changePassword(const QString &uname, const QString &newpass)
{
  auto recordList = getWhere("username = :uname", {{"uname", uname}}, "", 1);
  if (recordList.isEmpty())
    return false;
  if (recordList.count() > 1)
    return false;

  auto &am = AuthManager::instance();
  auto newSalt = am.generateSalt();
  auto newHash = am.generateHash(newpass, newSalt);

  QVariantMap param;
  param["salt"] = newSalt;
  param["password_hash"] = newHash;
  param["updated_at"] = QDateTime::currentDateTimeUtc();

  return update(recordList[0].value("id").toInt(), param);
}

bool AdminManager::changeUsername(const QString &old, const QString &newname)
{
  auto recordList = getWhere("username = :uname", {{"uname", old}}, "", 1);
  if (recordList.isEmpty())
    return false;
  if (recordList.count() > 1)
    return false;

  QVariantMap p;
  p["username"] = newname;
  p["updated_at"] = QDateTime::currentDateTimeUtc();
  return update(recordList[0].value("id").toInt(), p);
}

bool AdminManager::setActive(int id, bool what)
{
  if (!exists(id))
    return false;
  if (isActive(id) && what)
  {
    return true;
  }
  QVariantMap var;
  var["is_active"] = what;
  var["updated_at"] = QDateTime::currentDateTimeUtc();
  return update(id, var);
}

bool AdminManager::isActive(int id)
{
  auto opt = getById(id);
  if (opt)
  {
    return (*opt).value("is_active").toBool();
  }
  return false;
}

std::optional<QSqlRecord> AdminManager::getRecord(const QString &name) const
{
  auto q = baseQuery();
  q.prepare("SELECT * FROM admins WHERE username = :un");
  q.bindValue(":un", name);
  if (q.exec() && q.next())
  {
    return q.record();
  }
  return std::nullopt;
}