#include "paymentmanager.h"
#include "invoicemanager.h"
#include "transaksimanager.h"
#include "akuntransaksimanager.h"
#include "kategoritransaksimanager.h"
#include "src/utils/sqltransaction.h"

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
    auto opt_pay = getById(id);
    
    if (!opt_pay) {
      setErrorString("Data pembayaran tidak ditemukan");
      return false;
    }
    
    int adminId = verifiedByAdminId < 1 ? opt_pay->value("admin_id").toInt() : verifiedByAdminId;
    int akunId = opt_pay->value("akun_transaksi_id").toInt();
    
    AkunTransaksiManager atm;
    auto akunRecord = atm.getById(akunId);
    if (!akunRecord) {
        setErrorString("Akun transaksi tidak ditemukan");
        return false;
    }

    int amount      = opt_pay->value("amount").toInt();
    int saldoBefore = akunRecord->value("saldo").toInt();
    int saldoAfter  = saldoBefore + amount;

    // Update saldo akun
    if (!atm.updateSaldo(akunId, saldoAfter)) {
        setErrorString("Gagal update saldo akun: " + atm.errorString());
        return false;
    }
    
    // Set Kategori Pemasukan
    KategoriTransaksiManager katman;
    auto opt_kat = katman.getById(4); // ID Khusus Penjualan/Pembayaran Invoice
    if (!opt_kat) {
      setErrorString("Data Corrupt : \nKategori penjualan produk tidak ditemukan dalam database");
      return false;
    }
    
    // Catat di buku besar transaksi
    TransaksiManager tm;
    QVariantMap trxParams;
    trxParams["akun_id"]        = akunId;
    trxParams["admin_id"]       = adminId;
    trxParams["kategori_id"]    = opt_kat->value("id");
    trxParams["tipe"]           = opt_kat->value("tipe");
    trxParams["deskripsi"]      = QString("Pembayaran invoice #%1").arg(opt_pay->value("invoice_number").toString());
    trxParams["amount_before"]  = saldoBefore;
    trxParams["amount"]         = amount;
    trxParams["amount_after"]   = saldoAfter;
    trxParams["reference_type"] = "payment";
    trxParams["reference_id"]   = id;
    trxParams["tanggal"]        = dateToSql();

    if (!tm.create(trxParams)) {
        setErrorString("Gagal catat transaksi: " + tm.errorString());
        return false;
    }

    bool ok = update( id, {
      { "verified_at", dateTimeToSql()},
      { "verified_by", adminId },
      { "verification_status", "verified" },
    } );

    InvoiceManager iman;
    if ( !iman.recalculate(opt_pay->value("invoice_id").toInt()) ) {
      setErrorString("Gagal update data invoice: " + iman.errorString());
      return false;
    }
    return true;
}

bool PaymentManager::cancel(int id, int admin_id)
{
    QSqlQuery q(BaseManager::connection);
    q.prepare(R"-(
    UPDATE payments 
        SET verification_status = 'cancelled',
            updated_at = :updated_at, 
            verified_by = :verified_by 
      WHERE id = :id AND 
            verification_status <> 'cancelled'; )-"
    );
    q.bindValue(":updated_at", dateTimeToSql());
    q.bindValue(":verified_by", admin_id);
    q.bindValue(":id", id);
    if(!q.exec()) {
        setErrorString(q.lastError().text());
        return false;
    }
    return q.numRowsAffected() > 0;
}
