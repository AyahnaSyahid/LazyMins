#include "akuntransaksimanager.h"
#include "transaksimanager.h"
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

bool AkunTransaksiManager::opname(int id, int newSaldo)
{
    SqlTransaction t;
    auto q = baseQuery();
    q.prepare("UPDATE akun_transaksi SET saldo = :nsaldo WHERE id = :id");
    q.bindValue(":nsaldo", newSaldo);
    q.bindValue(":id", id);
    if (!q.exec()) {
        setErrorString(q.lastError().text());
        return false;
    }
    TransaksiManager trm;
    auto opt_tr = trm.create({
        {"akun_id", id},
        {"tipe", "opname"},
        {"deskripsi", "Opname"},
        {"amount_before", 0},
        {"amount", 0},
        {"amount_after", newSaldo},
        {"created_at", QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss")},
        {"updated_at", QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss")}
    });
}
