#include "rolesmanager.h"

std::optional<QSqlRecord> RolesManager::create(const QVariantMap &params)
{
  m_errorString = "";
  
  if(!params.contains("role_name") || !params.contains("description")) {
      m_errorString = "Parameter role_name atau description tidak ditemukan";
      return std::nullopt;
    }
  if (params.count() > 2) {
      m_errorString = "Parameter terlalu banyak";
      return std::nullopt;
    }
  return BaseManager::create(params);
}

std::optional<QString> RolesManager::roleName(int id) const {
  auto q = BaseManager::baseQuery();
  if (q.exec(QString("SELECT role_name FROM roles WHERE id = %1").arg(id)) && q.next()) {
    return q.value("role_name").toString();
  }
  return std::nullopt;
}

std::optional<int> RolesManager::roleId(const QString& roleName) const {
  auto q = BaseManager::baseQuery();
  q.prepare("SELECT id FROM roles WHERE role_name = :name");
  q.bindValue(":name", roleName);
  if(q.exec() && q.next()) {
      return q.value("id").toInt();
    }
  return std::nullopt;  
}
