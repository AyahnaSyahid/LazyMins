#include "productpricemanager.h"

// ============================================================================
// ProductPriceManager  (composite PK — bypasses id-based BaseManager methods)
// ============================================================================

std::optional<QSqlRecord> ProductPriceManager::getByCompositeKey(int productId, int priceLevelId) const
{
    QSqlQuery q(BaseManager::connection);
    q.prepare("SELECT * FROM product_prices WHERE product_id = :pid AND price_level_id = :plid");
    q.bindValue(":pid", productId);
    q.bindValue(":plid", priceLevelId);
    if (q.exec() && q.next()) return q.record();
    return std::nullopt;
}

bool ProductPriceManager::upsert(int productId, int priceLevelId, int price)
{
    QSqlQuery q(BaseManager::connection);
    q.prepare(
        "INSERT INTO product_prices (product_id, price_level_id, price, created_at, updated_at) "
        "VALUES (:pid, :plid, :price, :created_at, :updated_at) "
        "ON CONFLICT(product_id, price_level_id) DO UPDATE SET "
        "price = excluded.price, updated_at = excluded.updated_at");
    q.bindValue(":pid",        productId);
    q.bindValue(":plid",       priceLevelId);
    q.bindValue(":price",      price);
    q.bindValue(":created_at", dateTimeToSql());
    q.bindValue(":updated_at", dateTimeToSql());
    if (!q.exec()) {
        qDebug() << "ProductPriceManager::upsert error:" << q.lastError().text();
        return false;
    }
    return true;
}

bool ProductPriceManager::removeByCompositeKey(int productId, int priceLevelId)
{
    QSqlQuery q(BaseManager::connection);
    q.prepare("DELETE FROM product_prices WHERE product_id = :pid AND price_level_id = :plid");
    q.bindValue(":pid",  productId);
    q.bindValue(":plid", priceLevelId);
    if (!q.exec()) {
        qDebug() << "ProductPriceManager::removeByCompositeKey error:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

QList<QSqlRecord> ProductPriceManager::getByProduct(int productId)
{
    return getWhere("product_id = :product_id", {{"product_id", productId}});
}

QList<QSqlRecord> ProductPriceManager::getByPriceLevel(int priceLevelId)
{
    return getWhere("price_level_id = :price_level_id", {{"price_level_id", priceLevelId}});
}

std::optional<int> ProductPriceManager::getPrice(int productId, int priceLevelId) const
{
    auto rec = getByCompositeKey(productId, priceLevelId);
    if (rec) return rec->value("price").toInt();
    return std::nullopt;
}
