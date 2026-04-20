#include "akuntransaksimanager.h"
#include "transaksimanager.h"
#include "src/utils/sessionmanager.h"
#include "src/utils/sqltransaction.h"

AkunTransaksiManager::AkunTransaksiManager()
    : BaseManager("akun_transaksi", false)
{
}

std::optional<QSqlRecord> AkunTransaksiManager::getByKode(const QString& kode)
{
    auto results = getWhere(
        "kode = :kode COLLATE NOCASE",
        {{ "kode", kode }}
    );
    if (results.isEmpty()) return std::nullopt;
    return results.first();
}

QList<QSqlRecord> AkunTransaksiManager::getActive(const QString& orderBy, int limit)
{
    return getWhere("is_active = 1", {}, orderBy, limit);
}

bool AkunTransaksiManager::deactivate(int id)
{
    return update(id, {{ "is_active", 0 }});
}

bool AkunTransaksiManager::updateSaldo(int id, int newSaldo)
{
    return update(id, {{ "saldo", newSaldo }});
}


bool AkunTransaksiManager::opname(int id, int newSaldo, const QString& deskripsi)
{
    if (!SessionManager::instance().currentUser().has_value()){
        setErrorString("Tidak ada user aktif");
        return false;
    }
    int currentUserId = SessionManager::instance().currentUser()->value("id").toInt();

    SqlTransaction t;

    if(!t.started()) {
        setErrorString("Tidak dapat memulai transaksi database");
        return false;
    }

    auto q = baseQuery();
    
    q.prepare("SELECT saldo FROM akun_transaksi WHERE id = :id");
    q.bindValue(":id", id);
    if (!q.exec() || !q.next()) {
        setErrorString(q.lastError().text());
        return false;
    }

    auto oldSaldo = q.value("saldo").toInt();

    q.prepare("UPDATE akun_transaksi SET saldo = :nsaldo WHERE id = :id AND saldo != :nsaldo");
    q.bindValue(":nsaldo", newSaldo);
    q.bindValue(":id", id);
    if (!q.exec()) {
        setErrorString(q.lastError().text());
        return false;
    }
    if (q.numRowsAffected() == 0) return true; // tidak ada perubahan
    
    q.prepare("SELECT id FROM kategori_transaksi WHERE tipe = :tipe");
    q.bindValue(":tipe", "opname");
    if (!q.exec()) {
        setErrorString(q.lastError().text());
        return false;
    }

    if(!q.next()) {
        setErrorString("Kategori opname tidak ditemukan");
        return false;
    }

    auto kategori_id = q.value("id").toInt();

    TransaksiManager trm;
    auto opt_tr = trm.create({
        {"akun_id", id},
        {"tipe", "opname"},
        {"transaction_number", TransaksiManager::nextNumber()},
        {"kategori_id", kategori_id},
        {"admin_id", currentUserId},
        {"deskripsi", deskripsi},
        {"amount_before", oldSaldo},
        {"amount", newSaldo - oldSaldo},
        {"amount_after", newSaldo},
        {"created_at", QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss")},
        {"updated_at", QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss")}
    });
    if(!opt_tr.has_value()) {
        setErrorString(trm.errorString());
        return false;
    }
    return t.commit();
}

bool AkunTransaksiManager::deposit(int id, int nominal, const QString &deskripsi)
{
    if (!SessionManager::instance().currentUser().has_value()){
        setErrorString("Tidak ada user aktif");
        return false;
    }
    int currentUserId = SessionManager::instance().currentUser()->value("id").toInt();
    
    SqlTransaction t;
    if(!t.started()) {
        setErrorString("Tidak dapat memulai transaksi database");
        return false;   
    }

    auto q = baseQuery();
    q.prepare("SELECT saldo FROM akun_transaksi WHERE id = :id");
    q.bindValue(":id", id);
    if (!q.exec() || !q.next()) {
        setErrorString(q.lastError().text());
        return false;
    }

    int saldo = q.value("saldo").toInt();

    if (saldo + nominal < 0) {
        setErrorString("Saldo tidak mencukupi");
        return false;
    }

    q.prepare("UPDATE akun_transaksi SET saldo = saldo + :nominal WHERE id = :id");
    q.bindValue(":nominal", nominal);
    q.bindValue(":id", id);
    if (!q.exec()) {
        setErrorString(q.lastError().text());
        return false;
    }

    TransaksiManager trm;
    auto opt_tr = trm.create({
        {"akun_id", id},
        {"tipe", nominal > 0 ? "pemasukan" : "pengeluaran"},
        {"transaction_number", TransaksiManager::nextNumber()},
        {"admin_id", currentUserId},
        {"deskripsi", deskripsi},
        {"amount_before", saldo},
        {"amount", nominal},
        {"amount_after", saldo + nominal},
        {"created_at", QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss")},
        {"updated_at", QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss")}
    });
    if(!opt_tr.has_value()) {
        setErrorString(trm.errorString());
        return false;
    }
    return t.commit();
}
