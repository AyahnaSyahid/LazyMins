#include "paymentmanager.h"

#include "src/managers/financialledgerservice.h"
#include "src/utils/sessionmanager.h"
#include "src/utils/sqltransaction.h"

QString PaymentManager::nextNumber() {
  return generateCode("payments", "payment_number", "PYM-", 5, true);
}

PaymentManager::PaymentManager() : BaseManager("payments", false) {}

bool PaymentManager::beforeCreate(QVariantMap& params) {
  if (!params.contains("payment_number") ||
      params["payment_number"].toString().isEmpty()) {
    params["payment_number"] = nextNumber();
  }
  if (!params.contains("admin_id") || params["admin_id"].toInt() <= 0) {
    int ca = SessionManager::instance().currentUserId();
    if (ca > 0) {
      params["admin_id"] = ca;
    } else {
      setErrorString("Parameter admin_id tidak valid");
      return false;
    }
  }
  return true;
}

QList<QSqlRecord> PaymentManager::getByInvoice(int invoiceId) {
  return getWhere("invoice_id = :invoice_id", {{"invoice_id", invoiceId}},
                  "payment_date DESC");
}

QList<QSqlRecord> PaymentManager::getByStatus(
    const QString& verificationStatus) {
  return getWhere("verification_status = :status COLLATE NOCASE",
                  {{"status", verificationStatus}}, "payment_date DESC");
}

bool PaymentManager::verify(int id, int verifiedByAdminId) {
  auto opt_pay = getById(id);

  if (!opt_pay) {
    setErrorString("Data pembayaran tidak ditemukan");
    return false;
  }

  if (opt_pay->value("verification_status").toString() == "verified") {
    setErrorString("Pembayaran sudah diverifikasi");
    return false;
  }

  if (opt_pay->value("verification_status").toString() == "cancelled") {
    setErrorString("Pembayaran sudah dibatalkan");
    return false;
  }

  return update(id, {{"verification_status", "verified"},
                     {"verified_at", dateTimeToSql()},
                     {"verified_by", verifiedByAdminId},
                     {"updated_at", dateTimeToSql()}});
}

bool PaymentManager::cancel(int id, int admin_id) {
  auto opt_pay = getById(id);

  if (!opt_pay) {
    setErrorString("Data pembayaran tidak ditemukan");
    return false;
  }

  if (opt_pay->value("verification_status").toString() == "cancelled") {
    setErrorString("Pembayaran sudah dibatalkan");
    return false;
  }

  QSqlQuery q(BaseManager::connection);
  q.prepare(R"-(
    UPDATE payments 
        SET verification_status = 'cancelled',
            updated_at = :updated_at, 
            verified_by = :verified_by 
      WHERE id = :id AND 
            verification_status <> 'cancelled'; )-");
  q.bindValue(":updated_at", dateTimeToSql());
  q.bindValue(":verified_by", admin_id);
  q.bindValue(":id", id);
  if (!q.exec()) {
    setErrorString(q.lastError().text());
    return false;
  }
  if (q.numRowsAffected() > 0) {
    FinancialLedgerService flc;
    return flc.handlePayment(id);
  }
  return false;
}
