#pragma once
#include <QJsonObject>
#include "basemanager.h"

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================
class ProductCategoryManager;
class ProductManager;
class PriceLevelManager;
class ProductPriceManager;
class FinishingServiceManager;
class OrderManager;
class OrderItemManager;
class OrderItemFinishingManager;
class PaymentMethodManager;
class PaymentManager;
class KategoriTransaksiManager;
class TransaksiManager;
class StockMovementManager;
class ActivityLogManager;

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

protected:
    QVariantMap validateParams(const QVariantMap& params) override;
};

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
    bool adjustStock(int id, int delta, const QString& notes = "");

protected:
    QVariantMap validateParams(const QVariantMap& params) override;
};

// ============================================================================
// PriceLevelManager — tabel: price_levels
// ============================================================================
class PriceLevelManager : public BaseManager
{
public:
    explicit PriceLevelManager()
        : BaseManager("price_levels") {}

    std::optional<QSqlRecord> findByName(const QString& levelName);

protected:
    QVariantMap validateParams(const QVariantMap& params) override;
};

// ============================================================================
// ProductPriceManager — tabel: product_prices  (composite PK: product_id + price_level_id)
// ============================================================================
class ProductPriceManager : public BaseManager
{
public:
    explicit ProductPriceManager()
        : BaseManager("product_prices") {}

    // Composite-key operations (override single-id methods)
    std::optional<QSqlRecord> getByCompositeKey(int productId, int priceLevelId);
    bool upsert(int productId, int priceLevelId, int price);
    bool removeByCompositeKey(int productId, int priceLevelId);

    // Convenience
    QList<QSqlRecord> getByProduct(int productId);
    QList<QSqlRecord> getByPriceLevel(int priceLevelId);
    std::optional<int>  getPrice(int productId, int priceLevelId);

protected:
    QVariantMap validateParams(const QVariantMap& params) override;

private:
    // Disable id-based inherited methods — product_prices has no single 'id' PK
    using BaseManager::getById;
    using BaseManager::update;
    using BaseManager::remove;
    using BaseManager::exists;
};

// ============================================================================
// FinishingServiceManager — tabel: finishing_services
// ============================================================================
class FinishingServiceManager : public BaseManager
{
public:
    explicit FinishingServiceManager()
        : BaseManager("finishing_services") {}

    QList<QSqlRecord> getActive(const QString& orderBy = "name");
    std::optional<QSqlRecord> findByCode(const QString& code);

protected:
    QVariantMap validateParams(const QVariantMap& params) override;
};

// ============================================================================
// OrderManager — tabel: orders
// ============================================================================
class OrderManager : public BaseManager
{
public:
    explicit OrderManager()
        : BaseManager("orders") {}

    // Filters
    QList<QSqlRecord> getByStatus(const QString& status, const QString& orderBy = "order_date DESC");
    QList<QSqlRecord> getByPaymentStatus(const QString& paymentStatus);
    QList<QSqlRecord> getByCustomer(int customerId);
    QList<QSqlRecord> getByDateRange(const QDate& from, const QDate& to);
    QList<QSqlRecord> getPending();
    QList<QSqlRecord> getOverdue();

    std::optional<QSqlRecord> findByOrderNumber(const QString& orderNumber);

    // Status transitions
    bool updateStatus(int id, const QString& newStatus);
    bool cancel(int id);
    bool markCompleted(int id);

    // Number generation  (prefix from app_settings, e.g. "ORD")
    static QString generateOrderNumber(const QString& prefix = "ORD");

protected:
    QVariantMap validateParams(const QVariantMap& params) override;
    void beforeCreate(QVariantMap& params) override;
};

// ============================================================================
// OrderItemManager — tabel: order_items
// ============================================================================
class OrderItemManager : public BaseManager
{
public:
    explicit OrderItemManager()
        : BaseManager("order_items") {}

    QList<QSqlRecord> getByOrder(int orderId);
    
    bool removeByOrder(int orderId);

protected:
    QVariantMap validateParams(const QVariantMap& params) override;
};

// ============================================================================
// OrderItemFinishingManager — tabel: order_item_finishings
// ============================================================================
class OrderItemFinishingManager : public BaseManager
{
public:
    explicit OrderItemFinishingManager()
        : BaseManager("order_item_finishings") {}

    QList<QSqlRecord> getByOrderItem(int orderItemId);
    bool removeByOrderItem(int orderItemId);

protected:
    QVariantMap validateParams(const QVariantMap& params) override;
};

// ============================================================================
// PaymentMethodManager — tabel: payment_methods
// ============================================================================
class PaymentMethodManager : public BaseManager
{
public:
    explicit PaymentMethodManager()
        : BaseManager("payment_methods") {}

    QList<QSqlRecord> getActive();
    std::optional<QSqlRecord> findByCode(const QString& methodCode);

protected:
    QVariantMap validateParams(const QVariantMap& params) override;
};

// ============================================================================
// PaymentManager — tabel: payments
// ============================================================================
class PaymentManager : public BaseManager
{
public:
    explicit PaymentManager()
        : BaseManager("payments") {}

    QList<QSqlRecord> getByOrder(int orderId);
    QList<QSqlRecord> getByCustomer(int customerId);
    QList<QSqlRecord> getByStatus(const QString& status);
    QList<QSqlRecord> getByDateRange(const QDate& from, const QDate& to);

    std::optional<QSqlRecord> findByPaymentNumber(const QString& paymentNumber);

    bool verify(int id, int verifiedByAdminId);
    bool cancelPayment(int id);

    static QString generatePaymentNumber(const QString& prefix = "PAY");

protected:
    QVariantMap validateParams(const QVariantMap& params) override;
    void beforeCreate(QVariantMap& params) override;
};

// ============================================================================
// KategoriTransaksiManager — tabel: kategori_transaksi
// ============================================================================
class KategoriTransaksiManager : public BaseManager
{
public:
    explicit KategoriTransaksiManager()
        : BaseManager("kategori_transaksi") {}

    QList<QSqlRecord> getByTipe(const QString& tipe); // 'pemasukan' | 'pengeluaran'
    QList<QSqlRecord> getActive();
    QList<QSqlRecord> getRootCategories();
    QList<QSqlRecord> getChildren(int parentId);
    std::optional<QSqlRecord> findByNama(const QString& nama);

protected:
    QVariantMap validateParams(const QVariantMap& params) override;
};

// ============================================================================
// TransaksiManager — tabel: transaksi
// ============================================================================
class TransaksiManager : public BaseManager
{
public:
    explicit TransaksiManager()
        : BaseManager("transaksi") {}

    QList<QSqlRecord> getByTipe(const QString& tipe);
    QList<QSqlRecord> getByAdmin(int adminId);
    QList<QSqlRecord> getByDateRange(const QDate& from, const QDate& to);
    QList<QSqlRecord> getByKategori(int kategoriId);
    QList<QSqlRecord> getByReference(const QString& referenceType, int referenceId);

    // Aggregates
    qint64 sumByTipe(const QString& tipe, const QDate& from = QDate(), const QDate& to = QDate());

    static QString generateTransactionNumber(const QString& prefix = "TRX");

protected:
    QVariantMap validateParams(const QVariantMap& params) override;
    void beforeCreate(QVariantMap& params) override;
};

// ============================================================================
// StockMovementManager — tabel: stock_movements
// ============================================================================
class StockMovementManager : public BaseManager
{
public:
    explicit StockMovementManager()
        : BaseManager("stock_movements") {}

    QList<QSqlRecord> getByProduct(int productId, int limit = -1);
    QList<QSqlRecord> getByType(const QString& movementType); // 'in' | 'out' | 'adjustment'
    QList<QSqlRecord> getByDateRange(const QDate& from, const QDate& to);
    QList<QSqlRecord> getByReference(const QString& referenceType, int referenceId);

    // Convenience recorder
    std::optional<QSqlRecord> recordMovement(int productId,
                                             const QString& type,
                                             int quantity,
                                             int stockBefore,
                                             int stockAfter,
                                             int adminId,
                                             const QString& referenceType = "",
                                             int referenceId = -1,
                                             const QString& notes = "");

protected:
    QVariantMap validateParams(const QVariantMap& params) override;
};

// ============================================================================
// ActivityLogManager — tabel: activity_logs
// ============================================================================
class ActivityLogManager : public BaseManager
{
public:
    explicit ActivityLogManager()
        : BaseManager("activity_logs") {}

    QList<QSqlRecord> getByAdmin(int adminId, int limit = 100);
    QList<QSqlRecord> getByAction(const QString& action);
    QList<QSqlRecord> getByTable(const QString& tableName);
    QList<QSqlRecord> getByDateRange(const QDate& from, const QDate& to);

    // Convenience logger
    std::optional<QSqlRecord> log(int adminId,
                                  const QString& action,
                                  const QString& tableName = "",
                                  int recordId = -1,
                                  const QJsonObject& oldValue = {},
                                  const QJsonObject& newValue = {},
                                  const QString& ipAddress = "",
                                  const QString& userAgent = "");

protected:
    QVariantMap validateParams(const QVariantMap& params) override;
};
