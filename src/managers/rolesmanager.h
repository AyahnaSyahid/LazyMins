#pragma once

#include "basemanager.h"

class RolesManager : public BaseManager
{
public:
  RolesManager() : BaseManager("roles", false) {}
  std::optional<QSqlRecord> create(const QVariantMap& params) override;
  std::optional<QString> roleName(int id) const;
  std::optional<int> roleId(const QString& roleName) const;
private:
  QString m_errorString {};
};
