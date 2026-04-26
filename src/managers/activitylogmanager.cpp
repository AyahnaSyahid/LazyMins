#include "activitylogmanager.h"
#include <QJsonDocument>
#include <QJsonObject>

ActivityLogManager::ActivityLogManager()
    : BaseManager("activity_logs", false)
{
}

bool ActivityLogManager::log(int adminId,
                              const QString& action,
                              const QString& tableName,
                              int recordId,
                              const QVariantMap& oldVal,
                              const QVariantMap& newVal)
{
    auto toJson = [](const QVariantMap& map) -> QString {
        if (map.isEmpty()) return QString();
        return QString::fromUtf8(
            QJsonDocument(QJsonObject::fromVariantMap(map)).toJson(QJsonDocument::Compact)
        );
    };

    QVariantMap params;
    params["admin_id"]   = adminId > 0 ? QVariant(adminId) : QVariant();
    params["action"]     = action;
    params["table_name"] = tableName.isEmpty() ? QVariant() : QVariant(tableName);
    params["record_id"]  = recordId > 0  ? QVariant(recordId)  : QVariant();
    params["old_value"]  = toJson(oldVal);
    params["new_value"]  = toJson(newVal);

    auto result = create(params);
    return result.has_value();
}

// DIBLOKIR
bool ActivityLogManager::update(int id, const QVariantMap& params)
{
    Q_UNUSED(id)
    Q_UNUSED(params)
    setErrorString("Activity log bersifat immutable dan tidak dapat diubah.");
    return false;
}

// DIBLOKIR
bool ActivityLogManager::remove(int id)
{
    Q_UNUSED(id)
    setErrorString("Activity log bersifat immutable dan tidak dapat dihapus.");
    return false;
}

QList<QSqlRecord> ActivityLogManager::getByAdmin(int adminId, const QString& orderBy, int limit)
{
    return getWhere("admin_id = :admin_id",
                    {{ "admin_id", adminId }},
                    orderBy, limit);
}

QList<QSqlRecord> ActivityLogManager::getByAction(const QString& action, int limit)
{
    return getWhere("action = :action",
                    {{ "action", action }},
                    "created_at DESC", limit);
}

QList<QSqlRecord> ActivityLogManager::getByTable(const QString& tableName, int recordId)
{
    QString condition = "table_name = :table_name";
    QVariantMap bindings = {{ ":table_name", tableName }};
    if (recordId > 0) {
        condition += " AND record_id = :record_id";
        bindings["record_id"] = recordId;
    }
    return getWhere(condition, bindings, "created_at DESC");
}
