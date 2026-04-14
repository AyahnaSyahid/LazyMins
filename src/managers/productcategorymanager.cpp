#include "productcategorymanager.h"

ProductCategoryManager::ProductCategoryManager()
    : BaseManager("product_categories", false)
{
}

QList<QSqlRecord> ProductCategoryManager::getActive(const QString& orderBy, int limit)
{
    return getWhere("is_active = 1", {}, orderBy, limit);
}

bool ProductCategoryManager::deactivate(int id)
{
    return update(id, {{ "is_active", 0 }});
}
