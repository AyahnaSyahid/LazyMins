#pragma once
#include <QJsonObject>
#include "basemanager.h"
#include "paymentmanager.h"
#include "transaksimanager.h"
#include "akuntransaksimanager.h"
#include "transaksimanager.h"
#include "invoicemanager.h"
#include "orderitemfinishingmanager.h"
// #include "orderitemmanager.h"
// #include "ordermanager.h"

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
class KategoriTransaksiManager;
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
    std::optional<QSqlRecord> findByName(const QString& name);
    bool adjustStock(int id, qreal delta, const QString& notes = "");
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
    QList<QSqlRecord> getByInvoice(int id);
    QList<QSqlRecord> getPending();
    QList<QSqlRecord> getOverdue();

    std::optional<QSqlRecord> findByOrderNumber(const QString& orderNumber);

    // Status transitions
    bool updateStagingStatus(int id, const QString& newStatus);
    bool cancel(int id);
    bool markCompleted(int id);
    
    // Number generation  (prefix from app_settings, e.g. "ORD")
    static QString generateOrderNumber(const QString& prefix = "ORD");
    
    bool updateSubtotal(int order_id);

protected:
    bool beforeCreate(QVariantMap& params) override;
    bool beforeUpdate(int, QVariantMap& params) override;
    bool afterUpdate(int id, const QSqlRecord&, const QSqlRecord&);
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
    bool updateItemFinishingTotal(int orderId);

protected:
    bool afterCreate(const QSqlRecord& c) override;
    bool afterUpdate(int, const QSqlRecord&, const QSqlRecord& c) override;
    bool afterDelete(int, const QSqlRecord&) override;
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
                                             qreal quantity,
                                             qreal stockBefore,
                                             qreal stockAfter,
                                             int adminId,
                                             const QString& referenceType = "",
                                             int referenceId = -1,
                                             const QString& notes = "");

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
};