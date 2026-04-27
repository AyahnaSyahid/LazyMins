#include "invoicemanager.h"

#include "ordermanager.h"

InvoiceManager::InvoiceManager() : BaseManager("invoices", false) {}

QString InvoiceManager::nextNumber() {
  return generateCode("invoices", "invoice_number", "INV-", 5, true);
}

bool InvoiceManager::beforeCreate(QVariantMap& params) {
  if (!params.contains("invoice_number") ||
      params["invoice_number"].toString().isEmpty()) {
    params["invoice_number"] = nextNumber();
  }
  return true;
}

std::optional<QSqlRecord> InvoiceManager::getByNumber(
    const QString& invoiceNumber) const {
  auto results = const_cast<InvoiceManager*>(this)->getWhere(
      "invoice_number = :invoice_number", {{":invoice_number", invoiceNumber}});
  if (results.isEmpty()) return std::nullopt;
  return results.first();
}

QList<QSqlRecord> InvoiceManager::getByCustomer(int customerId,
                                                const QString& orderBy) {
  return getWhere("customer_id = :cid AND is_active = 1",
                  {{":cid", customerId}}, orderBy);
}

QList<QSqlRecord> InvoiceManager::getByStagingStatus(const QString& status) {
  return getWhere("staging_status = :status COLLATE NOCASE AND is_active = 1",
                  {{":status", status}}, "created_at DESC");
}

QList<QSqlRecord> InvoiceManager::getBySettlementStatus(const QString& status) {
  return getWhere(
      "settlement_status = :status COLLATE NOCASE AND is_active = 1",
      {{":status", status}}, "due_date");
}

QList<QSqlRecord> InvoiceManager::getActive(const QString& orderBy, int limit) {
  return getWhere("is_active = 1", {}, orderBy, limit);
}

bool InvoiceManager::updateStagingStatus(int id, const QString& status) {
  return update(id, {{"staging_status", status}});
}

bool InvoiceManager::updateSettlementStatus(int id, const QString& status) {
  return update(id, {{"settlement_status", status}});
}

bool InvoiceManager::updatePaidAmount(int id, int paidAmount) {
  auto record = getById(id);
  if (!record) {
    setErrorString("Invoice tidak ditemukan");
    return false;
  }

  int totalAmount = record->value("total_amount").toInt();

  QString newSettlementStatus;
  if (paidAmount <= 0)
    newSettlementStatus = "unpaid";
  else if (paidAmount >= totalAmount)
    newSettlementStatus = "paid";
  else
    newSettlementStatus = "partial";

  return update(id, {{"paid_amount", paidAmount},
                     {"settlement_status", newSettlementStatus}});
}

bool InvoiceManager::deactivate(int id) {
  return update(id, {{"is_active", 0}});
}

bool InvoiceManager::addOrders(int invoice_id, QList<int> oids) {
  if (oids.isEmpty()) {
    setErrorString("List Order kosong");
    return false;
  }

  OrderManager om;
  for (int oid : oids) {
    om.setInvoiceId(oid, invoice_id);
  }

  return recalculate(invoice_id);
}

bool InvoiceManager::addOrder(int invoice_id, int oid) {
  return addOrders(invoice_id, {oid});
}

bool InvoiceManager::removeOrders(int invoice_id, QList<int> oids) {
  if (oids.isEmpty()) return true;

  QStringList holders;
  for (int i = 0; i < oids.size(); ++i)
    holders << QString(":bind_%1").arg(i, 2, 10, QChar('0'));

  QString sql(R"-(
      UPDATE orders SET ( invoice_id, invoice_number, updated_at ) =
        ( NULL, NULL, CURRENT_TIMESTAMP ) WHERE invoice_id = :iid AND id IN (%1))-");

  auto q = baseQuery();

  q.prepare(sql.arg(holders.join(", ")));
  q.bindValue(":iid", invoice_id);

  for (int i = 0; i < oids.size(); ++i) {
    q.bindValue(holders[i], oids[i]);
  }

  if (!q.exec()) {
    setErrorString("Gagal memisahkan orders dari invoice" +
                   q.lastError().text());
    return false;
  }
  return recalculate(invoice_id);
}

bool InvoiceManager::removeOrder(int invoice_id, int oid) {
  return removeOrders(invoice_id, {oid});
}

bool InvoiceManager::recalculate(int id) {
  // Recusive Recalculate all_order
  QSqlQuery q(BaseManager::connection);
  q.prepare(R"-(
    UPDATE invoices
      SET subtotal = (
              SELECT COALESCE(SUM(total_amount), 0) 
                FROM orders
                WHERE invoice_id = :id AND
                      staging_status <> 'cancelled'
          )
    WHERE id = :id2;  
  )-");
  q.bindValue(":id", id);
  q.bindValue(":id2", id);
  if (!q.exec()) {
    setErrorString(q.lastError().text());
    return false;
  }
  return true;
}