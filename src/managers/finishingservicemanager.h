#pragma once
#include "basemanager.h"

class FinishingServiceManager : public BaseManager
{
public:
    explicit FinishingServiceManager();

    std::optional<QSqlRecord> getByCode(const QString& code) const;
    QList<QSqlRecord> getActive(const QString& orderBy = "name", int limit = -1);
    bool deactivate(int id);

protected:
    bool beforeCreate(QVariantMap& params) override;
};
