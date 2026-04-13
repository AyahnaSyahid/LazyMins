#pragma once

#include "basemanager.h"

// ============================================================================
// ProductCategoryManager — tabel: product_categories
// ============================================================================
class ProductCategoryManager : public BaseManager
{
public:
    explicit ProductCategoryManager()
        : BaseManager("product_categories") {}

    // Query helpers
    QList<QSqlRecord> getActive(const QString& orderBy = "category_name");
    std::optional<QSqlRecord> findByName(const QString& name);
};
