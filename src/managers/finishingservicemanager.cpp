#include "finishingservicemanager.h"

// ============================================================================
// FinishingServiceManager
// ============================================================================

QList<QSqlRecord> FinishingServiceManager::getActive(const QString& orderBy)
{
    return getWhere("is_active = 1", {}, orderBy);
}

std::optional<QSqlRecord> FinishingServiceManager::findByCode(const QString& code)
{
    auto rows = getWhere("code = :code", {{"code", code}});
    if (!rows.isEmpty()) return rows.first();
    return std::nullopt;
}
