#include "productmanager.h"

ProductManager::ProductManager()
    : BaseManager("products", false)
{
}

bool ProductManager::beforeCreate(QVariantMap &params)
{
    // Auto-generate SKU jika belum diisi
    if (!params.contains("sku") || params["sku"].toString().isEmpty())
    {
        params["sku"] = generateCode("products", "sku", "PRD-", 5);
    }
    return true;
}

std::optional<QSqlRecord> ProductManager::getBySku(const QString &sku) const
{
    auto results = const_cast<ProductManager *>(this)->getWhere(
        "sku = :sku COLLATE NOCASE",
        {{"sku", sku}});
    if (results.isEmpty())
        return std::nullopt;
    return results.first();
}

std::optional<QSqlRecord> ProductManager::getByName(const QString &name) const
{
    auto results = const_cast<ProductManager *>(this)->getWhere(
        "name = :name COLLATE NOCASE",
        {{"name", name}});
    if (results.isEmpty())
        return std::nullopt;
    return results.first();
}

QList<QSqlRecord> ProductManager::getByCategory(int categoryId)
{
    return getWhere("category_id = :cat_id AND is_active = 1",
                    {{"cat_id", categoryId}},
                    "name");
}

QList<QSqlRecord> ProductManager::getActive(const QString &orderBy, int limit)
{
    return getWhere("is_active = 1", {}, orderBy, limit);
}

QList<QSqlRecord> ProductManager::getLowStock()
{
    return getWhere("is_active = 1 AND stock <= min_stock", {}, "name");
}

bool ProductManager::deactivate(int id)
{
    return update(id, {{"is_active", 0}});
}

bool ProductManager::adjustStock(int id, double delta)
{
    auto record = getById(id);
    if (!record)
    {
        setErrorString("Product tidak ditemukan");
        return false;
    }
    double currentStock = record->value("stock").toDouble();
    return update(id, {{"stock", qCeil((currentStock + delta) * 100.0) / 100.0}, {"stock", currentStock + delta}, {"updated_at", dateTimeToSql()}});
}
