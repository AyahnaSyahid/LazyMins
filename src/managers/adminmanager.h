#pragma once

#include "basemanager.h"

class AdminManager : public BaseManager
{
  public:
    AdminManager() : BaseManager("admins", false) {}
    ~AdminManager() {}

    bool exists(const QString& username);
    using BaseManager::exists;
    
  protected:
    void beforeCreate(QVariantMap &m) override;
};
