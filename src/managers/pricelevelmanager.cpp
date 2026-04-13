#include "pricelevelmanager.h"

// ============================================================================
// PriceLevelManager
// ============================================================================

std::optional<QSqlRecord> PriceLevelManager::findByName(const QString& levelName)
{
    auto rows = getWhere("level_name = :level_name", {{"level_name", levelName}});
    if (!rows.isEmpty()) return rows.first();
    return std::nullopt;
}
