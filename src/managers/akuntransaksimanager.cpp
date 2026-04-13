// ============================================================================
// AkunTransaksiManager
// ============================================================================

#include "akuntransaksimanager.h"

QList<QSqlRecord> AkunTransaksiManager::getActive()
{
    return getWhere("is_active = 1", {}, "nama ASC");
}

std::optional<QSqlRecord> AkunTransaksiManager::findByKode(const QString& kode)
{
    auto rows = getWhere("kode = :kode", {{"kode", kode}});
    if (!rows.isEmpty()) return rows.first();
    return std::nullopt;
}

bool AkunTransaksiManager::updateSaldo(int id, qint64 newSaldo, int adminId)
{
    return update(id, {{"saldo", newSaldo}, {"admin_id", adminId} });
}