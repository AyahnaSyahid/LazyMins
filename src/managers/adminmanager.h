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

    bool changePassword(const QString& uname, const QString& newpass);
    bool changeUsername(const QString& oldname, const QString& newName);
  
  protected:
    void beforeCreate(QVariantMap &m) override;
    void beforeUpdate(int id, QVariantMap &m) override;
};
