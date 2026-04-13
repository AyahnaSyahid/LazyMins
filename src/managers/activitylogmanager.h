#pragma once

#include "basemanager.h"
#include <QJsonObject>

// ============================================================================
// ActivityLogManager — tabel: activity_logs
// ============================================================================
class ActivityLogManager : public BaseManager
{
public:
    explicit ActivityLogManager()
        : BaseManager("activity_logs") {}

    QList<QSqlRecord> getByAdmin(int adminId, int limit = 100);
    QList<QSqlRecord> getByAction(const QString& action);
    QList<QSqlRecord> getByTable(const QString& tableName);
    QList<QSqlRecord> getByDateRange(const QDate& from, const QDate& to);

    // Convenience logger
    std::optional<QSqlRecord> log(int adminId,
                                  const QString& action,
                                  const QString& tableName = "",
                                  int recordId = -1,
                                  const QJsonObject& oldValue = {},
                                  const QJsonObject& newValue = {},
                                  const QString& ipAddress = "",
                                  const QString& userAgent = "");
};