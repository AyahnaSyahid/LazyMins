#pragma once
#include "basemanager.h"

// ActivityLogManager bersifat append-only.
// update() dan remove() diblokir.
class ActivityLogManager : public BaseManager
{
public:
    explicit ActivityLogManager();

    // Helper: log aksi dengan format terstandar
    bool log(int adminId,
             const QString& action,
             const QString& tableName  = QString(),
             int recordId              = 0,
             const QVariantMap& oldVal = {},
             const QVariantMap& newVal = {});

    QList<QSqlRecord> getByAdmin(int adminId, const QString& orderBy = "created_at DESC", int limit = 100);
    QList<QSqlRecord> getByAction(const QString& action, int limit = 100);
    QList<QSqlRecord> getByTable(const QString& tableName, int recordId = -1);

    // DIBLOKIR — log bersifat immutable
    bool update(int id, const QVariantMap& params) override;
    bool remove(int id) override;
};
