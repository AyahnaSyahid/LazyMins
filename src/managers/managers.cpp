#include "managers.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QDate>


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
    q.bindValue(":updated_at", dateTimeToSql());
    q.bindValue(":id", id);
    if (!q.exec()) {
        qDebug() << "ProductManager::adjustStock error:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
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
        "deadline_date < :now AND staging_status NOT IN ('completed','cancelled')",
        {{"now", dateTimeToSql()}},
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
        p["completion_date"] = dateTimeToSql();
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

bool OrderManager::beforeCreate(QVariantMap& params)
{
    if (!params.contains("order_number") || params["order_number"].toString().isEmpty())
        params["order_number"] = generateOrderNumber();
    if (!params.contains("order_date"))
        params["order_date"] = dateTimeToSql();
    return true;
}

bool OrderManager::updateSubtotal(int order_id) {
  OrderItemManager oim;
  auto rList = oim.getByOrder(order_id);
  qint64 sum = 0;
  for(auto const& oi : rList) {
    sum += oi.value("total").toLongLong();
  }
  return update(order_id, {{"subtotal", sum},{"updated_at", dateTimeToSql()}});
}

bool OrderManager::beforeUpdate(int id, QVariantMap& params) {
  auto opt_od = getById(id);
  if(!opt_od.has_value()) {
    setErrorString("Order tidak ditemukan");
    return false;
  }
  auto recOd = *opt_od;
  if (!recOd.value("invoice_id").isNull() && params.contains("invoice_id")) {
    setErrorString("Mengupdate id invoice dalam order tidak diizinkan");
    return false;
  }
  return BaseManager::beforeUpdate(id, params);
}

bool OrderManager::afterUpdate(int id, const QSqlRecord& a, const QSqlRecord& b) {
  qint64 ttl_a, ttl_b;
  ttl_a = a.value("total_amount").toLongLong();
  ttl_b = b.value("total_amount").toLongLong();
  
  
  if (ttl_a != ttl_b) {
    if (!b.value("invoice_id").isNull()) {
      InvoiceManager im;
      if (!im.updateInvoiceBalances(b.value("invoice_id").toInt())) {
        setErrorString(im.errorString());
        return false;
      }
    }
  }
  return true;
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

bool OrderItemManager::updateItemFinishingTotal(int orderId) {
  OrderItemFinishingManager oifm;
  
  auto rlist = oifm.getByOrderItem(orderId);
  qint64 sum = 0;
  for (auto const& r : rlist) {
    sum += r.value("subtotal").toLongLong();
  }

  return update(orderId, {{"finishing_total", sum}, {"updated_at", dateTimeToSql()}});
}


bool OrderItemManager::afterCreate(const QSqlRecord& c) {
  return true;
}

bool OrderItemManager::afterUpdate(int, const QSqlRecord&, const QSqlRecord& c) {
  return true;
  
}

bool OrderItemManager::afterDelete(int, const QSqlRecord&) {
  return true;
  
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
    p["movement_date"]  = dateTimeToSql();
    if (!referenceType.isEmpty()) p["reference_type"] = referenceType;
    if (referenceId > 0)          p["reference_id"]   = referenceId;
    if (!notes.isEmpty())         p["notes"]          = notes;
    return create(p);
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