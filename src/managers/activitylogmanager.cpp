#include "activitylogmanager.h"
#include <QJsonDocument>

// ============================================================================
// ActivityLogManager
// ============================================================================

QList<QSqlRecord> ActivityLogManager::getByAdmin(int adminId, int limit)
{
    return getWhere("admin_id = :admin_id",
                    {{"admin_id", adminId}},
                    "created_at DESC", limit);
}

QList<QSqlRecord> ActivityLogManager::getByAction(const QString& action)
{
    return getWhere("action = :action", {{"action", action}}, "created_at DESC");
}

QList<QSqlRecord> ActivityLogManager::getByTable(const QString& tableName)
{
    return getWhere("table_name = :table_name", {{"table_name", tableName}}, "created_at DESC");
}

QList<QSqlRecord> ActivityLogManager::getByDateRange(const QDate& from, const QDate& to)
{
    return getWhere(
        "DATE(created_at) BETWEEN :from AND :to",
        {{"from", dateToSql(from)}, {"to", dateToSql(to)}},
        "created_at DESC");
}

std::optional<QSqlRecord> ActivityLogManager::log(
    int adminId, const QString& action,
    const QString& table, int recordId,
    const QJsonObject& oldValue, const QJsonObject& newValue,
    const QString& ipAddress, const QString& userAgent)
{
    QVariantMap p;
    p["admin_id"]  = adminId;
    p["action"]    = action;
    if (!table.isEmpty())       p["table_name"] = table;
    if (recordId > 0)           p["record_id"]  = recordId;
    if (!oldValue.isEmpty())    p["old_value"]  = QString(QJsonDocument(oldValue).toJson(QJsonDocument::Compact));
    if (!newValue.isEmpty())    p["new_value"]  = QString(QJsonDocument(newValue).toJson(QJsonDocument::Compact));
    if (!ipAddress.isEmpty())   p["ip_address"] = ipAddress;
    if (!userAgent.isEmpty())   p["user_agent"] = userAgent;
    return create(p);
}