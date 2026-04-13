#include "productcategorymanager.h"


// ============================================================================
// ProductCategoryManager
// ============================================================================

QList<QSqlRecord> ProductCategoryManager::getActive(const QString& orderBy)
{
    return getWhere("is_active = 1", {}, orderBy);
}

std::optional<QSqlRecord> ProductCategoryManager::findByName(const QString& name)
{
    auto rows = getWhere("category_name = :category_name",
                         {{"category_name", name}});
    if (!rows.isEmpty()) return rows.first();
    return std::nullopt;
}
