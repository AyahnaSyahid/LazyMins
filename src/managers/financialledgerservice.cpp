#include "financialledgerservice.h"

#include "src/managers/invoicemanager.h"
#include "src/managers/paymentmanager.h"
#include "src/managers/transaksimanager.h"
#include "src/utils/sessionmanager.h"
#include "src/utils/sqltransaction.h"

namespace {
void debugSqlRecord(const QSqlRecord& record) {
  if (record.isEmpty()) {
    qDebug() << "[DebugRecord] Record is empty.";
    return;
  }

  qDebug() << "--- [QSqlRecord Debug] ---";
  for (int i = 0; i < record.count(); ++i) {
    QString fieldName = record.fieldName(i);
    QVariant value = record.value(i);
    QString typeName = value.typeName();

    // Menangani tampilan jika nilainya NULL
    QString displayValue = value.isNull() ? "NULL" : value.toString();

    qDebug().noquote() << QString("[%1] %2 (%3) : %4")
                              .arg(i, 2)
                              .arg(fieldName, -20)
                              .arg(typeName, -10)
                              .arg(displayValue);
  }
  qDebug() << "---------------------------";
}
bool exq(QSqlQuery& q, QString& setError) {
  if (!q.exec()) {
    setError = q.lastError().text();
    qWarning() << setError;
    return false;
  }
  return true;
}
}  // namespace


bool FinancialLedgerService::handlePayment(int paymentId) {
  TransaksiManager trm;
  auto exec = [this](QSqlQuery& q) { return exq(q, this->m_errorString); };
  if (!canBeHandled(paymentId)) {
    m_errorString = "[FinancialLedgerService] Payment condition not satisfied";
    qWarning() << m_errorString;
    return false;
  }

  PaymentManager pm;
  auto optPay = pm.getById(paymentId);
  if (!optPay) {
    m_errorString = "[FinancialLedgerService] Payment not found";
    qWarning() << m_errorString;
    return false;
  }
  auto q = *optPay;

  QString deskripsi;
  QString vstat = q.value("verification_status").toString();
  bool cancel = false;
  // Gunakan pengecekan eksplisit
  if (vstat == "cancelled") {
    deskripsi = "[SystemLOG] Pembatalan Pembayaran Invoice #" +
                q.value("invoice_id").toString();
    cancel = true;
  } else if (vstat == "verified") {
    deskripsi =
        "[SystemLOG] Pembayaran Invoice #" + q.value("invoice_id").toString();
  } else {
    qInfo() << "[FinancialLedgerService] Pembayaran belum terverifikasi log di "
               "skip";
    return true;
  }

  {  // Update paid_amount & settlement_status pada tabel invoice
    InvoiceManager im;
    auto optInv = im.getById(q.value("invoice_id").toInt());
    if (!optInv) {
      m_errorString = "[FinancialLedgerService] Invoice not found";
      qWarning() << m_errorString;
      return false;
    }

    if (true) {
      debugSqlRecord(*optInv);
    }

    // Ambil data dari record invoice saat ini
    qint64 currentPaid = optInv->value("paid_amount").toLongLong();
    qint64 totalInvoice = optInv->value("total_amount").toLongLong();
    qint64 paymentAmount = q.value("amount").toLongLong();

    // 1. Kalkulasi Paid Amount Baru
    // Jika cancel: kurangi saldo terbayar. Jika verified: tambah saldo
    // terbayar.
    qint64 newPaid =
        cancel ? (currentPaid - paymentAmount) : (currentPaid + paymentAmount);

    // Safety check agar tidak terjadi underflow (paid_amount < 0)
    if (newPaid < 0) {
      m_errorString =
          "Invalid calculation: Paid amount cannot be less than zero";
      qWarning() << m_errorString;
      return false;
    }

    // 2. Tentukan Settlement Status secara manual berdasarkan perbandingan
    // nilai
    QString newStatus;
    if (newPaid <= 0) {
      newStatus = "unpaid";
    } else if (newPaid >= totalInvoice) {
      newStatus = "paid";
    } else {
      newStatus = "partial";
    }

    // 3. Eksekusi Update ke Database
    QSqlQuery iup(BaseManager::connection);
    iup.prepare(R"-(
            UPDATE invoices 
            SET paid_amount = :paid, 
                settlement_status = :status,
                updated_at = CURRENT_TIMESTAMP 
            WHERE id = :id
        )-");

    iup.bindValue(":paid", newPaid);
    iup.bindValue(":status", newStatus);
    iup.bindValue(":id", q.value("invoice_id").toInt());

    if (!exec(iup)) {
      m_errorString = "[FinancialLedgerService] Update Invoice Error: " +
                      iup.lastError().text();
      return false;
    }
  }

  QSqlQuery atrq(BaseManager::connection);
  atrq.prepare("SELECT * FROM akun_transaksi WHERE id = :akun_id");
  atrq.bindValue(":akun_id", q.value("akun_transaksi_id"));
  
  if (!exec(atrq)) {
    m_errorString = "[FinancialLedgerService] Akun Transaksi Error: " +
                    atrq.lastError().text();
    return false;
  }
  
  if (!atrq.next()) {
    m_errorString = "[FinancialLedgerService] Akun Transaksi not found";
    qWarning() << m_errorString;
    return false;
  }

  qint64 amountValue = cancel ? -q.value("amount").toLongLong()
                        : q.value("amount").toLongLong();
  auto optTr = trm.create({
      {"akun_id", q.value("akun_transaksi_id")},
      {"admin_id", SessionManager::instance().currentUserId()},
      {"kategori_id",
       FinancialLedgerService::KategoriTransaksi::PembayaranInvoice},
      {"tipe", cancel ? "pengeluaran" : "pemasukan"},
      {"deskripsi", deskripsi},
      {"amount", amountValue},
      {"payment_method", "AkunID #" + q.value("akun_transaksi_id").toString()},
      {"reference_type", "payments"},
      {"reference_id", paymentId},
      {"tanggal", q.value("verified_at")},
      {"amount_before", atrq.value("saldo")},
      {"amount_after", atrq.value("saldo").toLongLong() + amountValue},
  });

  if (!optTr.has_value()) {
    m_errorString = trm.errorString();
    qWarning() << m_errorString;
    return false;
  }

  auto q2 = QSqlQuery(BaseManager::connection);

  q2.prepare(
      "UPDATE akun_transaksi SET saldo = saldo + :after, updated_at = "
      "CURRENT_TIMESTAMP WHERE id = :akun_transaksi_id");
  q2.bindValue(":after", q.value("amount").toLongLong());
  q2.bindValue(":akun_transaksi_id", q.value("akun_transaksi_id").toInt());
  if (!exec(q2)) return false;
  return true;
}

bool FinancialLedgerService::canBeHandled(int paymentId) {
  QSqlQuery q(BaseManager::connection);
  auto exec = [this](QSqlQuery& q) { return exq(q, this->m_errorString); };
  QString query = R"-(
    SELECT py.akun_transaksi_id AS akun_id,
           py.invoice_id,
           py.verification_status,
           py.verified_by,
           py.verified_at,
           at.saldo AS before,
           py.amount,
           CASE WHEN py.verification_status = 'cancelled' THEN at.saldo - py.amount 
                WHEN py.verification_status = 'verified' THEN at.saldo + py.amount 
                ELSE at.saldo END AS after
      FROM payments py
           JOIN
           akun_transaksi at ON at.id = py.akun_transaksi_id
           JOIN
           invoices inv ON inv.id = py.invoice_id
     WHERE py.id = :payment_id AND
           inv.settlement_status <> 'paid' AND
           inv.is_active = 1 AND
           ( py.verification_status = 'cancelled' OR inv.remaining_amount >= py.amount); )-";
  q.prepare(query);
  q.bindValue(":payment_id", paymentId);

  if (!exec(q)) return false;
  return q.next();
}

bool FinancialLedgerService::isVerified(int paymentId) const {
  PaymentManager pym;
  auto opt_pay = pym.getById(paymentId);
  if (opt_pay.has_value()) {
    auto rc = *opt_pay;
    return (!rc.value("verified_by").isNull()) &&
           (rc.value("verified_by").toInt() > 0);
  }
  return false;
}

bool FinancialLedgerService::createPayment(const QVariantMap& params,
                                           int* paymentId) {
  SqlTransaction t;
  if (!t.started()) {
    m_errorString = "Gagal membuat transaksi Database";
    return false;
  }
  PaymentManager pm;
  auto optPayment = pm.create(params);
  if (!optPayment.has_value()) {
    m_errorString = pm.errorString();
    qWarning() << m_errorString;
    return false;
  }

  if (!handlePayment(optPayment->value("id").toInt())) {
    m_errorString = "Gagal membuat transaksi ke akun : " + m_errorString;
    return false;
  }
  if (!t.commit()) {
    m_errorString = "Gagal membuat transaksi Database";
    return false;
  }
  if (paymentId) *paymentId = optPayment->value("id").toInt();
  return true;
}

bool FinancialLedgerService::verify(int paymentId, int by) {
  if (isVerified(paymentId)) return false;
  PaymentManager pym;
  SqlTransaction tr;
  if (!tr.started()) {
    m_errorString = "Gagal membuat transaksi Database";
    return false;
  };
  if (!pym.verify(paymentId, by)) {
    m_errorString = pym.errorString();
    return false;
  };
  if (handlePayment(paymentId)) return tr.commit();
  return false;
}
