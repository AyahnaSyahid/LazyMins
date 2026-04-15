#include "akuntransaksimanager.h"

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
