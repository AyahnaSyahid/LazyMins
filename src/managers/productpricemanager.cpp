#include "productpricemanager.h"

ProductPriceManager::ProductPriceManager()
    : BaseManager("product_prices", false)
{
}

bool ProductPriceManager::upsert(int productId, int priceLevelId, int price)
{
    QSqlQuery q = baseQuery();
    q.prepare(
        "INSERT INTO product_prices (product_id, price_level_id, price, updated_at) "
        "VALUES (:product_id, :price_level_id, :price, :updated_at) "
        "ON CONFLICT(product_id, price_level_id) DO UPDATE SET "
        "  price      = excluded.price, "
        "  updated_at = excluded.updated_at"
    );
    q.bindValue(":product_id",    productId);
    q.bindValue(":price_level_id", priceLevelId);
    q.bindValue(":price",          price);
    q.bindValue(":updated_at",     dateTimeToSql());

    if (!q.exec()) {
        setErrorString(q.lastError().text());
        return false;
    }
    return true;
}

QList<QSqlRecord> ProductPriceManager::getByProduct(int productId)
{
    return getWhere("product_id = :product_id",
                    {{ ":product_id", productId }},
                    "price_level_id");
}

std::optional<QSqlRecord> ProductPriceManager::getPrice(int productId, int priceLevelId)
{
    auto results = getWhere(
        "product_id = :product_id AND price_level_id = :price_level_id",
        {{ ":product_id", productId }, { ":price_level_id", priceLevelId }}
    );
    if (results.isEmpty()) return std::nullopt;
    return results.first();
}

bool ProductPriceManager::removePrice(int productId, int priceLevelId)
{
    QSqlQuery q = baseQuery();
    q.prepare("DELETE FROM product_prices WHERE product_id = :pid AND price_level_id = :plid");
    q.bindValue(":pid",  productId);
    q.bindValue(":plid", priceLevelId);
    if (!q.exec()) {
        setErrorString(q.lastError().text());
        return false;
    }
    return true;
}

// Tidak relevan — tabel ini tidak punya kolom id tunggal
std::optional<QSqlRecord> ProductPriceManager::getById(int id) const
{
    qWarning() << "getById tidak didukung untuk ProductPriceManager. Gunakan getPrice(productId, priceLevelId).";
    return std::nullopt;
}

bool ProductPriceManager::remove(int id)
{
    Q_UNUSED(id)
    setErrorString("remove(id) tidak didukung untuk ProductPriceManager. Gunakan removePrice(productId, priceLevelId).");
    return false;
}
