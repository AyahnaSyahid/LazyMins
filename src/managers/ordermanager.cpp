#include "ordermanager.h"

#include "konsumenmanager.h"
#include "orderitemmanager.h"

QString OrderManager::nextNumber() {
  return generateCode("orders", "order_number", "ORD-", 5, true);
}

OrderManager::OrderManager() : BaseManager("orders", false) {}

bool OrderManager::beforeCreate(QVariantMap& params) {
  // Auto-generate order_number jika belum diisi
  if (!params.contains("order_number") ||
      params["order_number"].toString().isEmpty()) {
    params["order_number"] = nextNumber();
  }
  return true;
}

bool OrderManager::afterCreate(const QSqlRecord& record) {
  // Update last_seen pada konsumen
  int customerId = record.value("customer_id").toInt();
  if (customerId > 0) {
    KonsumenManager km;
    km.update(customerId, {{"last_seen", dateTimeToSql()}});
  }
  return true;
}

QList<QSqlRecord> OrderManager::getByCustomer(int customerId,
                                              const QString& orderBy) {
  return getWhere("customer_id = :cid", {{"cid", customerId}}, orderBy);
}

QList<QSqlRecord> OrderManager::getByStatus(const QString& status,
                                            const QString& orderBy) {
  return getWhere("staging_status = :status COLLATE NOCASE",
                  {{"status", status}}, orderBy);
}

QList<QSqlRecord> OrderManager::getByInvoice(int invoiceId) {
  return getWhere("invoice_id = :inv_id", {{"inv_id", invoiceId}});
}

bool OrderManager::updateStagingStatus(int id, const QString& status) {
  return update(id,
                {{"staging_status", status}, {"updated_at", dateTimeToSql()}});
}

bool OrderManager::setInvoiceId(int id, int invoiceId) {
  return update(id,
                {{"invoice_id", invoiceId}, {"updated_at", dateTimeToSql()}});
}

bool OrderManager::addItems(int id, QList<int> itemIds) {
  OrderItemManager oim;
  for (int itemId : itemIds) {
    oim.setOrderId(itemId, id);
  }
  return recalculate(id);
}

bool OrderManager::recalculate(int oid) {
  QSqlQuery q(BaseManager::connection);
  q.prepare(
      "UPDATE orders SET subtotal = ( SELECT COALESCE(SUM(total), 0) FROM "
      "order_items WHERE order_id = :oid ) WHERE id = :oid2");
  q.bindValue(":oid", oid);
  q.bindValue(":oid2", oid);
  if (!q.exec()) {
    setErrorString(q.lastError().text());
    return false;
  }
  return true;
}
