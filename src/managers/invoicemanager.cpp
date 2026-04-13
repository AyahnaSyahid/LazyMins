#include "invoicemanager.h"

// ============================================================================
// InvoiceManager
// ============================================================================

QList<QSqlRecord> InvoiceManager::getByStatus(const QString& status, const QString& orderBy)
{
    return getWhere("settlement_status = :status", {{"settlement_status", status}}, orderBy);
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
        "due_date < :now AND settlement_status NOT IN ('paid','cancelled')",
        {{"now", dateTimeToSql()}},
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
    QVariantMap p{{"settlement_status", newStatus}};
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

bool InvoiceManager::beforeCreate(QVariantMap& params)
{
    if (!params.contains("invoice_number") || params["invoice_number"].toString().isEmpty())
        params["invoice_number"] = generateInvoiceNumber();
    if (!params.contains("issue_date"))
        params["issue_date"] = dateTimeToSql();
    if (!params.contains("staging_status"))
        params["staging_status"] = "draft";
    return true;
}

bool InvoiceManager::updateInvoiceBalances(int invoiceId) {
  // 1. Hitung total bayar dari tabel payments (hanya yang statusnya 'verified')
    QSqlQuery q(BaseManager::connection);
    q.prepare("SELECT COALESCE(SUM(amount), 0) FROM payments "
              "WHERE invoice_id = :id AND verification_status = 'verified'");
    q.bindValue(":id", invoiceId);
    
    if (!q.exec() || !q.next()) return false;
    
    qint64 totalPaid = q.value(0).toLongLong();
    
    // 2. Ambil total tagihan invoice
    auto optInv = getById(invoiceId);
    if (!optInv) return false;
    qint64 total_amount = optInv->value("total_amount").toLongLong();
    
    // 3. Tentukan settlement_status
    QString status = "unpaid";
    if (totalPaid >= total_amount) status =  "paid";
    else if (totalPaid > 0)      status = "partial";
    
    // 4. Update tabel invoices
    return update(invoiceId, {
        {"paid_amount", totalPaid},
        {"settlement_status", status}
    });
}