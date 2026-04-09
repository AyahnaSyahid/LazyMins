#include "managers.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QDate>

// ============================================================================
// Shared helpers (file-local)
// ============================================================================
namespace {

// Generates a sequential number-based code: PREFIX-00001
// Uses COUNT(*)+1 on the given table (fast enough for percetakan scale)
QString generateCode(const QString& tableName,
                     const QString& numberColumn,
                     const QString& prefix,
                     int padWidth = 5)
{
    QSqlQuery q(BaseManager::connection);
    q.prepare(QString("SELECT COALESCE(MAX(%1), 0) + 1 AS next_val FROM %2")
                  .arg(numberColumn, tableName));
    if (q.exec() && q.next()) {
        int next = q.value("next_val").toInt();
        return QString("%1-%2").arg(prefix).arg(next, padWidth, 10, QChar('0'));
    }
    // Fallback: timestamp-based
    return QString("%1-%2").arg(prefix)
               .arg(QDateTime::currentMSecsSinceEpoch());
}

QString dateToSql(const QDate& d = QDateTime::currentDateTimeUtc().date()) { return d.toString("yyyy-MM-dd"); }
QString datetimeToSql(const QDateTime& d = QDateTime::currentDateTimeUtc()) { return d.toString("yyyy-MM-dd HH:mm:ss"); }

} // anonymous namespace

// ============================================================================
// ProductCategoryManager
// ============================================================================

QList<QSqlRecord> ProductCategoryManager::getActive(const QString& orderBy)
{
    return getWhere("is_active = 1", {}, orderBy);
}

std::optional<QSqlRecord> ProductCategoryManager::findByName(const QString& name)
{
    auto rows = getWhere("category_name = :category_name",
                         {{"category_name", name}});
    if (!rows.isEmpty()) return rows.first();
    return std::nullopt;
}

QVariantMap ProductCategoryManager::validateParams(const QVariantMap& params)
{
    QVariantMap p = params;
    // Strip fields that don't belong to the table
    for (const QString& key : p.keys()) {
        static const QStringList allowed {
            "category_name", "description", "is_active", "created_at", "updated_at"
        };
        if (!allowed.contains(key)) p.remove(key);
    }
    return p;
}

// ============================================================================
// ProductManager
// ============================================================================

QList<QSqlRecord> ProductManager::getActive(const QString& orderBy)
{
    return getWhere("is_active = 1", {}, orderBy);
}

QList<QSqlRecord> ProductManager::getByCategory(int categoryId)
{
    return getWhere("category_id = :category_id AND is_active = 1",
                    {{"category_id", categoryId}}, "name");
}

QList<QSqlRecord> ProductManager::getLowStock()
{
    return getWhere("is_active = 1 AND stock <= min_stock", {}, "name");
}

std::optional<QSqlRecord> ProductManager::findBySku(const QString& sku)
{
    auto rows = getWhere("sku = :sku COLLATE NOCASE", {{"sku", sku}});
    if (!rows.isEmpty()) return rows.first();
    return std::nullopt;
}
std::optional<QSqlRecord> ProductManager::findByName(const QString& name)
{
    auto rows = getWhere("name = :name COLLATE NOCASE", {{"name", name}});
    if (!rows.isEmpty()) return rows.first();
    return std::nullopt;
}

bool ProductManager::adjustStock(int id, qreal delta, const QString& notes)
{
    Q_UNUSED(notes) // caller should record a StockMovement separately
    QSqlQuery q(BaseManager::connection);
    q.prepare(QString("UPDATE %1 SET stock = stock + :delta, updated_at = :updated_at WHERE id = :id")
                  .arg(tableName()));
    q.bindValue(":delta", delta);
    q.bindValue(":updated_at", datetimeToSql());
    q.bindValue(":id", id);
    if (!q.exec()) {
        qDebug() << "ProductManager::adjustStock error:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

QVariantMap ProductManager::validateParams(const QVariantMap& params)
{
    QVariantMap p = params;
    static const QStringList allowed {
        "sku", "name", "category_id", "description", "unit", "use_area", 
        "stock", "min_stock", "cost_price", "use_area", "is_active", "created_at", "updated_at"
    };
    for (const QString& key : p.keys())
        if (!allowed.contains(key)) p.remove(key);
    return p;
}

// ============================================================================
// PriceLevelManager
// ============================================================================

std::optional<QSqlRecord> PriceLevelManager::findByName(const QString& levelName)
{
    auto rows = getWhere("level_name = :level_name", {{"level_name", levelName}});
    if (!rows.isEmpty()) return rows.first();
    return std::nullopt;
}

QVariantMap PriceLevelManager::validateParams(const QVariantMap& params)
{
    QVariantMap p = params;
    static const QStringList allowed {
        "level_name", "discount_percentage", "description", "created_at", "updated_at"
    };
    for (const QString& key : p.keys())
        if (!allowed.contains(key)) p.remove(key);
    return p;
}

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
    q.bindValue(":created_at", datetimeToSql());
    q.bindValue(":updated_at", datetimeToSql());
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

QVariantMap ProductPriceManager::validateParams(const QVariantMap& params)
{
    QVariantMap p = params;
    static const QStringList allowed { "product_id", "price_level_id", "price", "created_at", "updated_at" };
    for (const QString& key : p.keys())
        if (!allowed.contains(key)) p.remove(key);
    return p;
}

// ============================================================================
// FinishingServiceManager
// ============================================================================

QList<QSqlRecord> FinishingServiceManager::getActive(const QString& orderBy)
{
    return getWhere("is_active = 1", {}, orderBy);
}

std::optional<QSqlRecord> FinishingServiceManager::findByCode(const QString& code)
{
    auto rows = getWhere("code = :code", {{"code", code}});
    if (!rows.isEmpty()) return rows.first();
    return std::nullopt;
}

QVariantMap FinishingServiceManager::validateParams(const QVariantMap& params)
{
    QVariantMap p = params;
    static const QStringList allowed {
        "code", "name", "description", "price_per_unit", "unit", "is_active", "created_at", "updated_at"
    };
    for (const QString& key : p.keys())
        if (!allowed.contains(key)) p.remove(key);
    return p;
}

// ============================================================================
// OrderManager
// ============================================================================

QList<QSqlRecord> OrderManager::getByStatus(const QString& status, const QString& orderBy)
{
    return getWhere("staging_status = :status", {{"status", status}}, orderBy);
}

QList<QSqlRecord> OrderManager::getByPaymentStatus(const QString& paymentStatus)
{
    return getWhere("payment_status = :payment_status",
                    {{"payment_status", paymentStatus}}, "order_date DESC");
}

QList<QSqlRecord> OrderManager::getByCustomer(int customerId)
{
    return getWhere("customer_id = :customer_id",
                    {{"customer_id", customerId}}, "order_date DESC");
}

QList<QSqlRecord> OrderManager::getByDateRange(const QDate& from, const QDate& to)
{
    return getWhere(
        "DATE(order_date) BETWEEN :from AND :to",
        {{"from", dateToSql(from)}, {"to", dateToSql(to)}},
        "order_date DESC");
}

QList<QSqlRecord> OrderManager::getByInvoice(int invoice_id) {
  return getWhere(
    "invoice_id = :invoice_id", 
    {{"invoice_id", invoice_id}});
}


QList<QSqlRecord> OrderManager::getPending()
{
    return getByStatus("pending", "order_date ASC");
}

QList<QSqlRecord> OrderManager::getOverdue()
{
    return getWhere(
        "deadline_date < :now AND status NOT IN ('completed','cancelled')",
        {{"now", datetimeToSql()}},
        "deadline_date ASC");
}

std::optional<QSqlRecord> OrderManager::findByOrderNumber(const QString& orderNumber)
{
    auto rows = getWhere("order_number = :order_number", {{"order_number", orderNumber}});
    if (!rows.isEmpty()) return rows.first();
    return std::nullopt;
}

bool OrderManager::updateStagingStatus(int id, const QString& newStatus)
{
    QVariantMap p;
    p["staging_status"] = newStatus;
    if (newStatus == "completed")
        p["completion_date"] = datetimeToSql();
    return update(id, p);
}

bool OrderManager::cancel(int id)  { return updateStagingStatus(id, "cancelled"); }
bool OrderManager::markCompleted(int id) { return updateStagingStatus(id, "completed"); }

QString OrderManager::generateOrderNumber(const QString& prefix)
{
    auto q = baseQuery();
    q.prepare(QString("SELECT '%1-' || '%2-' || printf('%05d',COALESCE(COUNT(*), 0) + 1) AS next_val FROM orders WHERE date(created_at) = date('now')")
                        .arg(prefix, QDate::currentDate().toString("yyyyMMdd")));
    if(q.exec() && q.next()) {
      return q.value("next_val").toString();
    }
    return generateCode("orders", "id", prefix);
}

QVariantMap OrderManager::validateParams(const QVariantMap& params)
{
    QVariantMap p = params;
    static const QStringList allowed {
        "order_number", "invoice_id", "invoice_number", "customer_id", 
        "customer_name", "customer_phone", "price_level_id",
        "subtotal", "discount_amount",
        "staging_status", "priority", "order_date", "deadline_date", 
        "completion_date", "notes", "internal_notes", "admin_id",
        "created_at", "updated_at"
    };
    for (const QString& key : p.keys())
        if (!allowed.contains(key)) p.remove(key);
    return p;
}

void OrderManager::beforeCreate(QVariantMap& params)
{
    if (!params.contains("order_number") || params["order_number"].toString().isEmpty())
        params["order_number"] = generateOrderNumber();
    if (!params.contains("order_date"))
        params["order_date"] = datetimeToSql();
}

// ============================================================================
// OrderItemManager
// ============================================================================

QList<QSqlRecord> OrderItemManager::getByOrder(int orderId)
{
    return getWhere("order_id = :order_id", {{"order_id", orderId}});
}

bool OrderItemManager::removeByOrder(int orderId)
{
    QSqlQuery q(BaseManager::connection);
    q.prepare(QString("DELETE FROM %1 WHERE order_id = :order_id").arg(tableName()));
    q.bindValue(":order_id", orderId);
    return q.exec();
}

QVariantMap OrderItemManager::validateParams(const QVariantMap& params)
{
    QVariantMap p = params;
    static const QStringList allowed {
        "order_id", "product_id", "product_name", "sku", "quantity", "unit",
        "base_price", "sale_price", "discount_percentage", "discount_amount", "subtotal",
        "size_width", "size_height", "use_area", "finishing_total",  // ← NEW fields
        "notes", "created_at", "updated_at", "total"
    };
    for (const QString& key : p.keys())
        if (!allowed.contains(key)) p.remove(key);
    return p;
}

// ============================================================================
// OrderItemFinishingManager
// ============================================================================

QList<QSqlRecord> OrderItemFinishingManager::getByOrderItem(int orderItemId)
{
    return getWhere("order_item_id = :order_item_id", {{"order_item_id", orderItemId}});
}

bool OrderItemFinishingManager::removeByOrderItem(int orderItemId)
{
    QSqlQuery q(BaseManager::connection);
    q.prepare(QString("DELETE FROM %1 WHERE order_item_id = :order_item_id").arg(tableName()));
    q.bindValue(":order_item_id", orderItemId);
    return q.exec();
}

QVariantMap OrderItemFinishingManager::validateParams(const QVariantMap& params)
{
    QVariantMap p = params;
    static const QStringList allowed {
        "order_item_id", "finishing_id", "finishing_name", "quantity",
        "finishing_price", "subtotal", "created_at", "updated_at"
    };
    for (const QString& key : p.keys())
        if (!allowed.contains(key)) p.remove(key);
    return p;
}

// ============================================================================
// PaymentManager
// ============================================================================

QList<QSqlRecord> PaymentManager::getByOrder(int orderId)
{
    return getWhere("order_id = :order_id",
                    {{"order_id", orderId}}, "payment_date DESC");
}

QList<QSqlRecord> PaymentManager::getByCustomer(int customerId)
{
    return getWhere("customer_id = :customer_id",
                    {{"customer_id", customerId}}, "payment_date DESC");
}

QList<QSqlRecord> PaymentManager::getByStatus(const QString& status)
{
    return getWhere("payment_status = :payment_status",
                    {{"payment_status", status}}, "payment_date DESC");
}

QList<QSqlRecord> PaymentManager::getByDateRange(const QDate& from, const QDate& to)
{
    return getWhere(
        "DATE(payment_date) BETWEEN :from AND :to",
        {{"from", dateToSql(from)}, {"to", dateToSql(to)}},
        "payment_date DESC");
}

std::optional<QSqlRecord> PaymentManager::findByPaymentNumber(const QString& paymentNumber)
{
    auto rows = getWhere("payment_number = :payment_number", {{"payment_number", paymentNumber}});
    if (!rows.isEmpty()) return rows.first();
    return std::nullopt;
}

bool PaymentManager::verify(int id, int verifiedByAdminId)
{
    return update(id, {
        {"verification_status", "verified"},
        {"verified_by",    verifiedByAdminId},
        {"verified_at",    datetimeToSql()}
    });
}

bool PaymentManager::cancelPayment(int id)
{
    return update(id, {{"verification_status", "cancelled"}});
}

QString PaymentManager::generatePaymentNumber(const QString& prefix)
{
    auto q = baseQuery();
    q.prepare(QString("SELECT '%1-' || '%2-' || printf('%05d',COALESCE(COUNT(*), 0) + 1) AS next_val "
                      "FROM payments WHERE date(payment_date) = date('now')")
                  .arg(prefix, QDate::currentDate().toString("yyyyMMdd")));
    if (q.exec() && q.next()) {
        return q.value("next_val").toString();
    }
    return generateCode("payments", "id", prefix);
}

QVariantMap PaymentManager::validateParams(const QVariantMap& params)
{
    QVariantMap p = params;
    static const QStringList allowed {
        "payment_number", "invoice_id", "amount", 
        "akun_transaksi_id", // Menggantikan payment_method
        "cash_received", "cash_change",
        "verification_status", "notes", "admin_id", 
        "payment_date", "verified_by", "verified_at",
        "created_at", "updated_at"
    };
    for (const QString& key : p.keys())
        if (!allowed.contains(key)) p.remove(key);
    return p;
}

void PaymentManager::beforeCreate(QVariantMap& params)
{
    if (!params.contains("payment_number") || params["payment_number"].toString().isEmpty())
        params["payment_number"] = generatePaymentNumber();
    if (!params.contains("payment_date"))
        params["payment_date"] = datetimeToSql();
}

// ============================================================================
// KategoriTransaksiManager
// ============================================================================

QList<QSqlRecord> KategoriTransaksiManager::getByTipe(const QString& tipe)
{
    return getWhere("tipe = :tipe AND is_active = 1", {{"tipe", tipe}}, "nama");
}

QList<QSqlRecord> KategoriTransaksiManager::getActive()
{
    return getWhere("is_active = 1", {}, "nama");
}

QList<QSqlRecord> KategoriTransaksiManager::getRootCategories()
{
    return getWhere("parent_id IS NULL AND is_active = 1", {}, "nama");
}

QList<QSqlRecord> KategoriTransaksiManager::getChildren(int parentId)
{
    return getWhere("parent_id = :parent_id", {{"parent_id", parentId}}, "nama");
}

std::optional<QSqlRecord> KategoriTransaksiManager::findByNama(const QString& nama)
{
    auto rows = getWhere("nama = :nama", {{"nama", nama}});
    if (!rows.isEmpty()) return rows.first();
    return std::nullopt;
}

QVariantMap KategoriTransaksiManager::validateParams(const QVariantMap& params)
{
    QVariantMap p = params;
    static const QStringList allowed {
        "kode", "nama", "tipe", "parent_id", "description", "is_active", "created_at", "updated_at"
    };
    for (const QString& key : p.keys())
        if (!allowed.contains(key)) p.remove(key);
    return p;
}

// ============================================================================
// TransaksiManager
// ============================================================================

QList<QSqlRecord> TransaksiManager::getByTipe(const QString& tipe)
{
    return getWhere("tipe = :tipe", {{"tipe", tipe}}, "tanggal DESC");
}

QList<QSqlRecord> TransaksiManager::getByAdmin(int adminId)
{
    return getWhere("admin_id = :admin_id", {{"admin_id", adminId}}, "tanggal DESC");
}

QList<QSqlRecord> TransaksiManager::getByDateRange(const QDate& from, const QDate& to)
{
    return getWhere(
        "tanggal BETWEEN :from AND :to",
        {{"from", dateToSql(from)}, {"to", dateToSql(to)}},
        "tanggal DESC");
}

QList<QSqlRecord> TransaksiManager::getByKategori(int kategoriId)
{
    return getWhere("kategori_id = :kategori_id", {{"kategori_id", kategoriId}}, "tanggal DESC");
}

QList<QSqlRecord> TransaksiManager::getByReference(const QString& referenceType, int referenceId)
{
    return getWhere(
        "reference_type = :rt AND reference_id = :rid",
        {{"rt", referenceType}, {"rid", referenceId}});
}

std::optional<QSqlRecord> TransaksiManager::lastTransaction() const {
  QSqlQuery q("SELECT * FROM transaksi ORDER BY id DESC LIMIT 1", BaseManager::connection);
  if(!q.next()) {
    return std::nullopt;
  }
  return q.record();
}

qint64 TransaksiManager::sumByTipe(const QString& tipe, const QDate& from, const QDate& to)
{
    QSqlQuery q(BaseManager::connection);
    QString sql = "SELECT COALESCE(SUM(jumlah), 0) AS total FROM transaksi WHERE tipe = :tipe";
    if (from.isValid() && to.isValid())
        sql += " AND tanggal BETWEEN :from AND :to";
    q.prepare(sql);
    q.bindValue(":tipe", tipe);
    if (from.isValid() && to.isValid()) {
        q.bindValue(":from", dateToSql(from));
        q.bindValue(":to",   dateToSql(to));
    }
    if (q.exec() && q.next())
        return q.value("total").toLongLong();
    return 0;
}

QString TransaksiManager::generateTransactionNumber(const QString& prefix)
{
  auto q = baseQuery();
  q.prepare(QString("SELECT '%1-' || '%2-' || printf('%05d',COALESCE(COUNT(*), 0) + 1) AS next_val "
                    "FROM transaksi WHERE date(tanggal) = date('now')")
                .arg(prefix, QDate::currentDate().toString("yyyyMMdd")));
  if (q.exec() && q.next()) {
    return q.value("next_val").toString();
  }
  return generateCode("transaksi", "id", prefix);
}

QVariantMap TransaksiManager::validateParams(const QVariantMap& params)
{
    QVariantMap p = params;
    static const QStringList allowed {
        "transaction_number", "admin_id", "kategori_id",
        "tipe", "deskripsi", "amount_before", "amount", "amount_after", "akun_id",
        "payment_method", "reference_type", "reference_id", "attachment",
        "tanggal", "created_at", "updated_at"
    };
    for (const QString& key : p.keys())
        if (!allowed.contains(key)) p.remove(key);
    return p;
}

void TransaksiManager::beforeCreate(QVariantMap& params)
{
    if (!params.contains("transaction_number") || params["transaction_number"].toString().isEmpty())
        params["transaction_number"] = generateTransactionNumber();
    if (!params.contains("tanggal"))
        params["tanggal"] = QDate::currentDate().toString("yyyy-MM-dd");
}

// ============================================================================
// StockConsumesRepo
// ============================================================================

QList<QSqlRecord> StockConsumesRepo::getByProduct(int productId, int limit)
{
    return getWhere("product_id = :product_id",
                    {{"product_id", productId}},
                    "consumes_date DESC", limit);
}

QList<QSqlRecord> StockConsumesRepo::getByType(const QString& movementType)
{
    return getWhere("movement_type = :movement_type",
                    {{"movement_type", movementType}}, "consumes_date DESC");
}

QList<QSqlRecord> StockConsumesRepo::getByDateRange(const QDate& from, const QDate& to)
{
    return getWhere(
        "DATE(movement_date) BETWEEN :from AND :to",
        {{"from", dateToSql(from)}, {"to", dateToSql(to)}},
        "consumes_date DESC");
}

QList<QSqlRecord> StockConsumesRepo::getByReference(const QString& referenceType, int referenceId)
{
    return getWhere(
        "reference_type = :rt AND reference_id = :rid",
        {{"rt", referenceType}, {"rid", referenceId}});
}

std::optional<QSqlRecord> StockConsumesRepo::recordConsumes(
    int productId, const QString& type, int quantity,
    int stockBefore, int stockAfter, int adminId,
    const QString& referenceType, int referenceId, const QString& notes)
{
    QVariantMap p;
    p["product_id"]     = productId;
    p["movement_type"]  = type;
    p["quantity"]       = quantity;
    p["stock_before"]   = stockBefore;
    p["stock_after"]    = stockAfter;
    p["admin_id"]       = adminId;
    p["consumes_date"]  = datetimeToSql();
    if (!referenceType.isEmpty()) p["reference_type"] = referenceType;
    if (referenceId > 0)          p["reference_id"]   = referenceId;
    if (!notes.isEmpty())         p["notes"]          = notes;
    return create(p);
}

QVariantMap StockConsumesRepo::validateParams(const QVariantMap& params)
{
    QVariantMap p = params;
    static const QStringList allowed {
        "product_id", "movement_type", "quantity",
        "stock_before", "stock_after",
        "reference_type", "reference_id",
        "notes", "admin_id", "consumes_date",
        "created_at", "updated_at"
    };
    for (const QString& key : p.keys())
        if (!allowed.contains(key)) p.remove(key);
    return p;
}



// ============================================================================
// StockMovementManager
// ============================================================================

QList<QSqlRecord> StockMovementManager::getByProduct(int productId, int limit)
{
    return getWhere("product_id = :product_id",
                    {{"product_id", productId}},
                    "movement_date DESC", limit);
}

QList<QSqlRecord> StockMovementManager::getByType(const QString& movementType)
{
    return getWhere("movement_type = :movement_type",
                    {{"movement_type", movementType}}, "movement_date DESC");
}

QList<QSqlRecord> StockMovementManager::getByDateRange(const QDate& from, const QDate& to)
{
    return getWhere(
        "DATE(movement_date) BETWEEN :from AND :to",
        {{"from", dateToSql(from)}, {"to", dateToSql(to)}},
        "movement_date DESC");
}

QList<QSqlRecord> StockMovementManager::getByReference(const QString& referenceType, int referenceId)
{
    return getWhere(
        "reference_type = :rt AND reference_id = :rid",
        {{"rt", referenceType}, {"rid", referenceId}});
}

std::optional<QSqlRecord> StockMovementManager::recordMovement(
    int productId, const QString& type, qreal quantity,
    qreal stockBefore, qreal stockAfter, int adminId,
    const QString& referenceType, int referenceId, const QString& notes)
{
    QVariantMap p;
    p["product_id"]     = productId;
    p["movement_type"]  = type;
    p["quantity"]       = quantity;
    p["stock_before"]   = stockBefore;
    p["stock_after"]    = stockAfter;
    p["admin_id"]       = adminId;
    p["movement_date"]  = datetimeToSql();
    if (!referenceType.isEmpty()) p["reference_type"] = referenceType;
    if (referenceId > 0)          p["reference_id"]   = referenceId;
    if (!notes.isEmpty())         p["notes"]          = notes;
    return create(p);
}

QVariantMap StockMovementManager::validateParams(const QVariantMap& params)
{
    QVariantMap p = params;
    static const QStringList allowed {
        "product_id", "movement_type", "quantity",
        "stock_before", "stock_after",
        "reference_type", "reference_id",
        "notes", "admin_id", "movement_date",
        "created_at", "updated_at"
    };
    for (const QString& key : p.keys())
        if (!allowed.contains(key)) p.remove(key);
    return p;
}

// ============================================================================
// ActivityLogManager
// ============================================================================

QList<QSqlRecord> ActivityLogManager::getByAdmin(int adminId, int limit)
{
    return getWhere("admin_id = :admin_id",
                    {{"admin_id", adminId}},
                    "created_at DESC", limit);
}

QList<QSqlRecord> ActivityLogManager::getByAction(const QString& action)
{
    return getWhere("action = :action", {{"action", action}}, "created_at DESC");
}

QList<QSqlRecord> ActivityLogManager::getByTable(const QString& tableName)
{
    return getWhere("table_name = :table_name", {{"table_name", tableName}}, "created_at DESC");
}

QList<QSqlRecord> ActivityLogManager::getByDateRange(const QDate& from, const QDate& to)
{
    return getWhere(
        "DATE(created_at) BETWEEN :from AND :to",
        {{"from", dateToSql(from)}, {"to", dateToSql(to)}},
        "created_at DESC");
}

std::optional<QSqlRecord> ActivityLogManager::log(
    int adminId, const QString& action,
    const QString& table, int recordId,
    const QJsonObject& oldValue, const QJsonObject& newValue,
    const QString& ipAddress, const QString& userAgent)
{
    QVariantMap p;
    p["admin_id"]  = adminId;
    p["action"]    = action;
    if (!table.isEmpty())       p["table_name"] = table;
    if (recordId > 0)           p["record_id"]  = recordId;
    if (!oldValue.isEmpty())    p["old_value"]  = QString(QJsonDocument(oldValue).toJson(QJsonDocument::Compact));
    if (!newValue.isEmpty())    p["new_value"]  = QString(QJsonDocument(newValue).toJson(QJsonDocument::Compact));
    if (!ipAddress.isEmpty())   p["ip_address"] = ipAddress;
    if (!userAgent.isEmpty())   p["user_agent"] = userAgent;
    return create(p);
}

QVariantMap ActivityLogManager::validateParams(const QVariantMap& params)
{
    QVariantMap p = params;
    static const QStringList allowed {
        "admin_id", "action", "table_name", "record_id",
        "old_value", "new_value", "ip_address", "user_agent",
        "created_at", "updated_at"
    };
    for (const QString& key : p.keys())
        if (!allowed.contains(key)) p.remove(key);
    return p;
}

// ============================================================================
// InvoiceManager
// ============================================================================

QList<QSqlRecord> InvoiceManager::getByStatus(const QString& status, const QString& orderBy)
{
    return getWhere("status = :status", {{"status", status}}, orderBy);
}

QList<QSqlRecord> InvoiceManager::getByCustomer(int customerId)
{
    return getWhere("customer_id = :customer_id",
                    {{"customer_id", customerId}}, "issue_date DESC");
}

QList<QSqlRecord> InvoiceManager::getByDateRange(const QDate& from, const QDate& to)
{
    return getWhere(
        "DATE(issue_date) BETWEEN :from AND :to",
        {{"from", dateToSql(from)}, {"to", dateToSql(to)}},
        "issue_date DESC");
}

QList<QSqlRecord> InvoiceManager::getOverdue()
{
    return getWhere(
        "due_date < :now AND status NOT IN ('paid','cancelled')",
        {{"now", datetimeToSql()}},
        "due_date ASC");
}

std::optional<QSqlRecord> InvoiceManager::findByInvoiceNumber(const QString& invoiceNumber)
{
    auto rows = getWhere("invoice_number = :invoice_number", {{"invoice_number", invoiceNumber}});
    if (!rows.isEmpty()) return rows.first();
    return std::nullopt;
}

bool InvoiceManager::updateStatus(int id, const QString& newStatus)
{
    QVariantMap p{{"status", newStatus}};
    return update(id, p);
}

bool InvoiceManager::cancel(int id)  { return updateStatus(id, "cancelled"); }
bool InvoiceManager::markPaid(int id) { return updateStatus(id, "paid"); }

QString InvoiceManager::generateInvoiceNumber(const QString& prefix)
{
    auto q = baseQuery();
    q.prepare(QString("SELECT '%1-' || '%2-' || printf('%05d',COALESCE(COUNT(*), 0) + 1) AS next_val "
                      "FROM invoices WHERE date(issue_date) = date('now')")
                  .arg(prefix, QDate::currentDate().toString("yyyyMMdd")));
    if (q.exec() && q.next()) {
        return q.value("next_val").toString();
    }
    return generateCode("invoices", "id", prefix);
}

QVariantMap InvoiceManager::validateParams(const QVariantMap& params)
{
    QVariantMap p = params;
    static const QStringList allowed {
        "invoice_number", "customer_id", "customer_name", "customer_phone", 
        "price_level_id", "admin_id", "subtotal", "discount_amount", 
        "tax_amount", "paid_amount",
        "staging_status", "settlement_status", "revision_no", "parent_id", 
        "is_active", "issue_date", "due_date", "notes", "internal_notes",
        "created_at", "updated_at"
    };
    for (const QString& key : p.keys())
        if (!allowed.contains(key)) p.remove(key);
    return p;
}

void InvoiceManager::beforeCreate(QVariantMap& params)
{
    if (!params.contains("invoice_number") || params["invoice_number"].toString().isEmpty())
        params["invoice_number"] = generateInvoiceNumber();
    if (!params.contains("issue_date"))
        params["issue_date"] = datetimeToSql();
    if (!params.contains("staging_status"))
        params["staging_status"] = "draft";
}

// ============================================================================
// AkunTransaksiManager
// ============================================================================

QList<QSqlRecord> AkunTransaksiManager::getActive()
{
    return getWhere("is_active = 1", {}, "nama ASC");
}

std::optional<QSqlRecord> AkunTransaksiManager::findByKode(const QString& kode)
{
    auto rows = getWhere("kode = :kode", {{"kode", kode}});
    if (!rows.isEmpty()) return rows.first();
    return std::nullopt;
}

bool AkunTransaksiManager::updateSaldo(int id, qint64 newSaldo)
{
    return update(id, {{"saldo", newSaldo}});
}

QVariantMap AkunTransaksiManager::validateParams(const QVariantMap& params)
{
    QVariantMap p = params;
    // Daftar kolom sesuai dengan percetakan_schema_improved.sql
    static const QStringList allowed {
        "kode", "nama", "tipe", "nama_bank", "nomor_rekening", 
        "atas_nama", "saldo", "is_active", "description", 
        "created_at", "updated_at"
    };
    
    for (const QString& key : p.keys()) {
        if (!allowed.contains(key)) {
            p.remove(key);
        }
    }
    return p;
}