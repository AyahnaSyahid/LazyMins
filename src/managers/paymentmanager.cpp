// ============================================================================
// PaymentManager
// ============================================================================

#include "managers.h"

QList<QSqlRecord> PaymentManager::getByInvoice(int invoiceId)
{
    return getWhere("invoice_id = :invoice_id",
                    {{"invoice_id", invoiceId}}, "payment_date DESC");
}

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
        {"verified_at",    dateTimeToSql()}
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
                      "FROM payments WHERE date(payment_date, 'localtime') = date('now', 'localtime')")
                  .arg(prefix, QDate::currentDate().toString("yyyyMMdd")));
    if (q.exec() && q.next()) {
        return q.value("next_val").toString();
    }
    return generateCode("payments", "id", prefix);
}

bool PaymentManager::beforeCreate(QVariantMap& params)
{
    if (!params.contains("payment_number") || params["payment_number"].toString().isEmpty())
        params["payment_number"] = generatePaymentNumber();
    if (!params.contains("payment_date"))
        params["payment_date"] = dateTimeToSql();
    return true;
}

bool PaymentManager::beforeUpdate(int id, QVariantMap& params) {
  Q_UNUSED(id);
  params.remove("id");
  params.remove("payment_number");
  return true;
};

bool PaymentManager::afterCreate(const QSqlRecord& rec) {
  
  // update akun transaksi
  int     akun_id        = rec.value("akun_transaksi_id").toInt();
  int     admin_id       = rec.value("admin_id").toInt();
  int     payment_id     = rec.value("id").toInt();
  QString payment_date     = rec.value("payment_date").toString();
  qint64  payment_amount = rec.value("amount").toLongLong();
  
  AkunTransaksiManager akunTransaksiManager;
  auto opt_trAccount = akunTransaksiManager.getById(akun_id);
  
  if (!opt_trAccount.has_value()) {
    QString err("Gagal menemukan Akun Transaksi");
    qWarning() << "[PaymentManager::afterCreate]" << err;
    setErrorString(err);
    return false;
  }
  
  auto trAccount    = *opt_trAccount;
  qint64 saldo_akun = trAccount.value("saldo").toLongLong();
  
  if( rec.value("verification_status").toString() == "verified" ) {
    // ini harus memicu akun untuk update saldonya di TransaksiManager::afterCreate
    TransaksiManager transaksiManager;
    KategoriTransaksiManager kategoriTransaksiManager;
    
    auto opt_kat = kategoriTransaksiManager.findByNama("Penjualan Produk");
    
    if (!opt_kat.has_value()) {
        QString err("Gagal menemukan Kategori Transaksi Penjualan");
        qWarning() << "[PaymentManager::afterCreate]" << err;
        setErrorString(err);
        return false;
    }
    
    auto kat = *opt_kat;
    
    QVariantMap tr_param {
      {"akun_id", akun_id},
      {"transaction_number", TransaksiManager::generateTransactionNumber() },
      {"admin_id", admin_id },
      {"kategori_id", kat.value("id").toInt() },
      {"tipe", kat.value("tipe").toString() },
      {"amount_before", trAccount.value("saldo") },
      {"amount", payment_amount },
      {"amount_after", saldo_akun + payment_amount },
      {"reference_type", "payments" },
      {"reference_id", payment_id },
      {"tanggal", payment_date }
    };
    
    auto opt_trx = transaksiManager.create(tr_param);
    if( !opt_trx.has_value() ) {
      setErrorString(transaksiManager.errorString());
      return false;
    }
  }

  return true;
}

bool PaymentManager::afterUpdate(int id, const QSqlRecord& before, const QSqlRecord& after) {
    // ... (Variabel yang sudah Anda definisikan) ...
    QString vstat_before = before.value("verification_status").toString();
    QString vstat_after  = after.value("verification_status").toString();
    qint64 amount_before = before.value("amount").toLongLong();
    qint64 amount_after  = after.value("amount").toLongLong();
    
    int akun_id         = after.value("akun_transaksi_id").toInt();
    int admin_id        = after.value("verified_by").toInt();
    int payment_id      = after.value("id").toInt();
    int invoice_after   = after.value("invoice_id").toInt(); 
    int invoice_before  = before.value("invoice_id").toInt();
    QString today       = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    TransaksiManager trxMgr;
    KategoriTransaksiManager katMgr;
    InvoiceManager invMgr; // Pastikan instance ini tersedia
    QVariantMap tr_param;

    auto opt_kat = katMgr.findByNama("Penjualan Produk");
    int kat_id = opt_kat ? opt_kat->value("id").toInt() : 0;

    // --- CASE 1: PERUBAHAN STATUS ---
    if (vstat_before != vstat_after) {
        if (vstat_before == "pending" && vstat_after == "verified") {
            tr_param = {
                {"akun_id", akun_id},
                {"transaction_number", TransaksiManager::generateTransactionNumber()},
                {"admin_id", admin_id},
                {"kategori_id", kat_id},
                {"tipe", "Masuk"},
                {"deskripsi", "Verifikasi Pembayaran: " + after.value("payment_number").toString()},
                {"amount", amount_after},
                {"reference_type", "payments"},
                {"reference_id", payment_id},
                {"tanggal", today}
            };
        }
        else if (vstat_before == "verified" && (vstat_after == "pending" || vstat_after == "cancelled")) {
            tr_param = {
                {"akun_id", akun_id},
                {"transaction_number", TransaksiManager::generateTransactionNumber()},
                {"admin_id", admin_id},
                {"kategori_id", kat_id},
                {"tipe", "Keluar"},
                {"deskripsi", "Reversal/Pembatalan Pembayaran: " + after.value("payment_number").toString()},
                {"amount", -amount_before}, 
                {"reference_type", "payments"},
                {"reference_id", payment_id},
                {"tanggal", today}
            };
        }
    }
    // --- CASE 2: STATUS TETAP VERIFIED, TAPI NOMINAL ATAU INVOICE BERUBAH ---
    else if (vstat_after == "verified" && (amount_before != amount_after || invoice_before != invoice_after)) {
        qint64 selisih = amount_after - amount_before;
        
        // Logika Transaksi: Jika hanya invoice yang berubah tapi nominal tetap, 
        // kita tidak perlu buat TRX baru (karena uang di akun transaksi tetap sama).
        // Namun jika nominal berubah, kita catat selisihnya.
        if (selisih != 0) {
            tr_param = {
                {"akun_id", akun_id},
                {"transaction_number", TransaksiManager::generateTransactionNumber()},
                {"admin_id", admin_id},
                {"kategori_id", kat_id},
                {"tipe", selisih > 0 ? "Masuk" : "Keluar"},
                {"deskripsi", "Koreksi Nominal: " + after.value("payment_number").toString()},
                {"amount", selisih},
                {"reference_type", "payments"},
                {"reference_id", payment_id},
                {"tanggal", today}
            };
        }
    }

    // --- EKSEKUSI PERUBAHAN ---

    // 1. Eksekusi Transaksi (jika ada perubahan finansial)
    if (!tr_param.isEmpty()) {
        if (!trxMgr.create(tr_param).has_value()) {
            setErrorString("Gagal membuat transaksi: " + trxMgr.errorString());
            return false;
        }
    }

    // 2. Update Saldo Invoice (Selalu lakukan jika statusnya Verified atau ada perubahan invoice/nominal)
    // Kita cek apakah perlu update invoice:
    if (vstat_after == "verified" || vstat_before == "verified") {
        
        // Update invoice saat ini
        if (!invMgr.updateInvoiceBalances(invoice_after)) {
            setErrorString("Gagal update invoice baru: " + invMgr.errorString());
            return false;
        }

        // Jika invoice berubah, update juga invoice yang lama
        if (invoice_before != invoice_after && invoice_before > 0) {
            if (!invMgr.updateInvoiceBalances(invoice_before)) {
                setErrorString("Gagal update invoice lama: " + invMgr.errorString());
                return false;
            }
        }
    }

    return true;
}