#pragma once

#include "basemanager.h"

// ============================================================================
// ProductManager — tabel: products
// ============================================================================
class ProductManager : public BaseManager
{
public:
    explicit ProductManager()
        : BaseManager("products") {}

    // Query helpers
    QList<QSqlRecord> getActive(const QString& orderBy = "name");
    QList<QSqlRecord> getByCategory(int categoryId);
    QList<QSqlRecord> getLowStock();
    std::optional<QSqlRecord> findBySku(const QString& sku);
    std::optional<QSqlRecord> findByName(const QString& name);
    bool adjustStock(int id, qreal delta, const QString& notes = "");
};