#include "ordermanager.h"
#include "orderitemmanager.h"
#include "invoicemanager.h"

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
