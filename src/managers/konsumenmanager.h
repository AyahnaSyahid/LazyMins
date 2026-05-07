#pragma once
#include "basemanager.h"

class KonsumenManager : public BaseManager
{
public:
    explicit KonsumenManager();

    std::optional<QSqlRecord> getByCode(const QString& customerCode) const;
    QList<QSqlRecord> getActive(const QString& orderBy = "nama_lengkap", int limit = -1);
    bool deactivate(int id);
    bool hasAnyRef(int id);

protected:
    bool beforeCreate(QVariantMap& params) override;
};
