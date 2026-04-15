#include "paymentmanager.h"
#include "invoicemanager.h"
#include "transaksimanager.h"
#include "akuntransaksimanager.h"

QString PaymentManager::nextNumber() {
  return generateCode("payments", "payment_number", "PYM-", 5, true);
}

PaymentManager::PaymentManager()
    : BaseManager("payments", false)
{
}

bool PaymentManager::beforeCreate(QVariantMap& params)
{
    if (!params.contains("payment_number") || params["payment_number"].toString().isEmpty()) {
        params["payment_number"] = nextNumber();
    }
    return true;
}

bool PaymentManager::afterCreate(const QSqlRecord& record)
{
    int invoiceId       = record.value("invoice_id").toInt();
    int amount          = record.value("amount").toInt();
    int akunId          = record.value("akun_transaksi_id").toInt();
    int adminId         = record.value("admin_id").toInt();
    int paymentId       = record.value("id").toInt();

    // 1. Hitung total paid dari semua payment verified/pending pada invoice ini
    QSqlQuery q = baseQuery();
    
    // tidak perlu mencatat jika belum verified
    if (record.value("verification_status").toString() != "verified") return true; 
    
    // 2. Catat transaksi kas melalui AkunTransaksiManager + TransaksiManager
    AkunTransaksiManager atm;
    auto akunRecord = atm.getById(akunId);
    if (!akunRecord) {
        setErrorString("Akun transaksi tidak ditemukan");
        return false;
    }

    int saldoBefore = akunRecord->value("saldo").toInt();
    int saldoAfter  = saldoBefore + amount;

    // Update saldo akun
    if (!atm.updateSaldo(akunId, saldoAfter)) {
        setErrorString("Gagal update saldo akun: " + atm.errorString());
        return false;
    }

    // Catat di buku besar transaksi
    TransaksiManager tm;
    QVariantMap trxParams;
    trxParams["akun_id"]        = akunId;
    trxParams["admin_id"]       = adminId;
    trxParams["tipe"]           = "pemasukan";
    trxParams["deskripsi"]      = QString("Pembayaran invoice #%1").arg(invoiceId);
    trxParams["amount_before"]  = saldoBefore;
    trxParams["amount"]         = amount;
    trxParams["amount_after"]   = saldoAfter;
    trxParams["reference_type"] = "payment";
    trxParams["reference_id"]   = paymentId;
    trxParams["tanggal"]        = dateToSql();

    if (!tm.create(trxParams)) {
        setErrorString("Gagal catat transaksi: " + tm.errorString());
        return false;
    }
    
    InvoiceManager iman;
    if ( !iman.recalculate(invoiceId) ) {
      setErrorString("Gagal update data invoice: " + iman.errorString());
      return false;
    }
    
    return true;
}

QList<QSqlRecord> PaymentManager::getByInvoice(int invoiceId)
{
    return getWhere("invoice_id = :invoice_id",
                    {{ ":invoice_id", invoiceId }},
                    "payment_date DESC");
}

QList<QSqlRecord> PaymentManager::getByStatus(const QString& verificationStatus)
{
    return getWhere("verification_status = :status COLLATE NOCASE",
                    {{ ":status", verificationStatus }},
                    "payment_date DESC");
}

bool PaymentManager::verify(int id, int verifiedByAdminId)
{
    return update(id, {
        { "verification_status", "verified" },
        { "verified_by",         verifiedByAdminId },
        { "verified_at",         dateTimeToSql() }
    });
}

bool PaymentManager::cancel(int id)
{
    return update(id, {{ "verification_status", "cancelled" }});
}
