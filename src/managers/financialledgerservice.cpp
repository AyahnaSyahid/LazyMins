#include "financialledgerservice.h"
#include "src/managers/transaksimanager.h"
#include "src/utils/sessionmanager.h"

namespace {
    bool exq(QSqlQuery &q, QString &setError) {
        if(!q.exec()) {
            setError = q.lastError().text();
            qWarning() << setError;
            return false;
        }
        return true;
    }
}

bool FinancialLedgerService::handlePayment(int paymentId)
{
    TransaksiManager trm;
    QSqlQuery q(BaseManager::connection);

    auto exec = [this](QSqlQuery &q) { return exq(q, this->m_errorString) ; };
    QString query = R"-(
    SELECT py.akun_transaksi_id AS akun_id,
           py.invoice_id,
           py.verification_status,
           py.verified_by,
           py.verified_at,
           at.saldo AS before,
           py.amount,
           CASE WHEN py.verification_status = 'cancelled' THEN at.saldo - py.amount WHEN py.verification_status = 'verified' THEN at.saldo + py.amount ELSE at.saldo END AS after
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
    if (!q.next()) {
        m_errorString = "[FinancialLedgerService] Payment condition not satisfied";
        qWarning() << m_errorString;
        return false;
    }
    
    QString deskripsi;
    QString vstat = q.value("verification_status").toString();
    bool cancel = false;

    // Gunakan pengecekan eksplisit
    if (vstat == "cancelled") {
        deskripsi = "[SystemLOG] Pembatalan Pembayaran Invoice #" + q.value("invoice_id").toString();
        cancel = true;
    } else if (vstat == "verified") {
        deskripsi = "[SystemLOG] Pembayaran Invoice #" + q.value("invoice_id").toString();
    } else {
        qInfo() << "[FinancialLedgerService] Pembayaran belum terverifikasi log di skip";
        return true;
    }

    auto optTr = trm.create( {
        {"akun_id", q.value("akun_id")},
        {"admin_id", SessionManager::instance().currentUserId() },
        {"kategori_id", FinancialLedgerService::KategoriTransaksi::PembayaranInvoice },
        {"tipe", cancel ? "pengeluaran" : "pemasukan"},
        {"deskripsi", deskripsi},
        {"amount", cancel ? -q.value("amount").toLongLong() : q.value("amount").toLongLong() },
        {"payment_method", "AkunID #" + q.value("akun_id").toString()},
        {"reference_type", "payments"},
        {"reference_id", paymentId },
        {"tanggal", q.value("verified_at")},
        {"amount_before", q.value("before").toLongLong()},
        {"amount_after", q.value("after").toLongLong()},
    });
    
    if (!optTr.has_value()) {
        m_errorString = trm.errorString();
        qWarning() << m_errorString;
        return false;
    }

    auto q2 = QSqlQuery(BaseManager::connection);

    q2.prepare("UPDATE akun_transaksi SET saldo = :after, updated_at = CURRENT_TIMESTAMP WHERE id = :akun_id");
    q2.bindValue(":after", q.value("after").toLongLong());
    q2.bindValue(":akun_id", q.value("akun_id").toInt());
    
    if (!exec(q2)) return false;

    return true;
}