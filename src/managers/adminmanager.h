#pragma once

#include "basemanager.h"

class AdminManager : public BaseManager
{
  public:
    AdminManager() : BaseManager("admins", false) {}
    ~AdminManager() {}
    
    std::optional<QSqlRecord> create(const QVariantMap& param) override;
    bool exists(const QString& username);
    bool hasRole() const;
    using BaseManager::exists;
    
  protected:
    void beforeCreate(QVariantMap& params) override;
};
