#pragma once
#include "basemanager.h"

struct Product {
    int id;
    QString sku;
    QString name;
    int category_id;
    QString description;
    QString unit;
    qreal stock;
    qreal minStock;
    qreal cost_price;
    bool use_area;
    bool is_active;
    QDateTime created_at;
    QDateTime updated_at;
};

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

    static Product fromRecord(const QSqlRecord& rec);
    bool save(Product& product);

protected:
    bool beforeCreate(QVariantMap& params) override;
};
