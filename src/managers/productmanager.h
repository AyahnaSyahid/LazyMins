#pragma once
#include "basemanager.h"

class ProductManager : public BaseManager
{
public:
    explicit ProductManager();

    std::optional<QSqlRecord> getBySku(const QString& sku) const;
    std::optional<QSqlRecord> getByName(const QString& name) const;
    QList<QSqlRecord> getByCategory(int categoryId);
    QList<QSqlRecord> getActive(const QString& orderBy = "name", int limit = -1);
    QList<QSqlRecord> getLowStock();
    bool deactivate(int id);
    bool adjustStock(int id, double delta);

protected:
    bool beforeCreate(QVariantMap& params) override;
};
