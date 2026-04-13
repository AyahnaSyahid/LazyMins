#include "kategoritransaksimanager.h"

// ============================================================================
// KategoriTransaksiManager
// ============================================================================

QList<QSqlRecord> KategoriTransaksiManager::getByTipe(const QString& tipe)
{
    return getWhere("tipe = :tipe AND is_active = 1", {{"tipe", tipe}}, "nama");
}

QList<QSqlRecord> KategoriTransaksiManager::getActive()
{
    return getWhere("is_active = 1", {}, "nama");
}

QList<QSqlRecord> KategoriTransaksiManager::getRootCategories()
{
    return getWhere("parent_id IS NULL AND is_active = 1", {}, "nama");
}

QList<QSqlRecord> KategoriTransaksiManager::getChildren(int parentId)
{
    return getWhere("parent_id = :parent_id", {{"parent_id", parentId}}, "nama");
}

std::optional<QSqlRecord> KategoriTransaksiManager::findByNama(const QString& nama)
{
    auto rows = getWhere("nama = :nama", {{"nama", nama}});
    if (!rows.isEmpty()) return rows.first();
    return std::nullopt;
}