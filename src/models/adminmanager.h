#ifndef ADMINMANAGER_H
#define ADMINMANAGER_H

#include "basemanager.h"

class AdminManager : public BaseManager
{
  public:
    AdminManager() : BaseManager("admins", false) {}
    ~AdminManager() {}
    
    QSqlRecord create(const QVariantMap& param) override;
    bool exists(const QString& username);
    using BaseManager::exists;
    
  protected:
    void beforeCreate(QVariantMap& params) override;
};

#endif