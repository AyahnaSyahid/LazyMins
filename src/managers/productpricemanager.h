#pragma once
#include "basemanager.h"

// Tabel product_prices memiliki PK composite (product_id, price_level_id)
// sehingga beberapa method BaseManager di-override untuk menyesuaikan.
class ProductPriceManager : public BaseManager
{
public:
    explicit ProductPriceManager();

    // Upsert: INSERT or REPLACE berdasarkan (product_id, price_level_id)
    bool upsert(int productId, int priceLevelId, int price);

    // Ambil semua harga untuk satu produk
    QList<QSqlRecord> getByProduct(int productId);

    // Ambil harga spesifik
    std::optional<QSqlRecord> getPrice(int productId, int priceLevelId);

    // Hapus berdasarkan composite key
    bool removePrice(int productId, int priceLevelId);

    // --- Override: tidak relevan untuk tabel composite key ---
    std::optional<QSqlRecord> getById(int id) const override;
    bool remove(int id) override;
};
