#include "pricelevelmanager.h"

PriceLevelManager::PriceLevelManager()
    : BaseManager("price_levels", false)
{
}

std::optional<QSqlRecord> PriceLevelManager::getByName(const QString& levelName) const
{
    auto results = const_cast<PriceLevelManager*>(this)->getWhere(
        "level_name = :level_name COLLATE NOCASE",
        {{ ":level_name", levelName }}
    );
    if (results.isEmpty()) return std::nullopt;
    return results.first();
}
