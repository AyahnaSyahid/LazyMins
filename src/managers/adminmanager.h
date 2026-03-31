#pragma once

#include "basemanager.h"

class AdminManager : public BaseManager
{
  public:
    AdminManager() : BaseManager("admins", false) {}
    ~AdminManager() {}

    bool exists(const QString& username);
    using BaseManager::exists;

    bool setActive(int id, bool state);
    bool isActive(int id);

    void setLastLog(std::optional<QSqlRecord>&);
    bool changePassword(const QString& uname, const QString& newpass);
    bool changeUsername(const QString& oldname, const QString& newName);
    bool changeLoginInfo(const QString& oldname, const QString& newName, const QString& newPass);
    bool userHasRole(int userId, const QString& roleName) const;
    std::optional<QSqlRecord> getRecord(const QString& name) const;
    
  protected:
    void beforeCreate(QVariantMap &m) override;
    void beforeUpdate(int id, QVariantMap &m) override;
};
