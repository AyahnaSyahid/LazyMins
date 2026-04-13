#pragma once

#include "basemanager.h"

// ============================================================================
// ProductPriceManager — tabel: product_prices  (composite PK: product_id + price_level_id)
// ============================================================================
class ProductPriceManager : public BaseManager
{
public:
    explicit ProductPriceManager()
        : BaseManager("product_prices") {}

    // Composite-key operations (override single-id methods)
    std::optional<QSqlRecord> getByCompositeKey(int productId, int priceLevelId) const;
    bool upsert(int productId, int priceLevelId, int price);
    bool removeByCompositeKey(int productId, int priceLevelId);

    // Convenience
    QList<QSqlRecord> getByProduct(int productId);
    QList<QSqlRecord> getByPriceLevel(int priceLevelId);
    std::optional<int>  getPrice(int productId, int priceLevelId) const;

private:
    // Disable id-based inherited methods — product_prices has no single 'id' PK
    using BaseManager::getById;
    using BaseManager::update;
    using BaseManager::remove;
    using BaseManager::exists;
};
