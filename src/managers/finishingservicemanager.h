#pragma once

#include "basemanager.h"

// ============================================================================
// FinishingServiceManager — tabel: finishing_services
// ============================================================================
class FinishingServiceManager : public BaseManager
{
public:
    explicit FinishingServiceManager()
        : BaseManager("finishing_services") {}

    QList<QSqlRecord> getActive(const QString& orderBy = "name");
    std::optional<QSqlRecord> findByCode(const QString& code);
};
