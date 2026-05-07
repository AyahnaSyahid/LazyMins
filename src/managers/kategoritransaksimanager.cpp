#include "kategoritransaksimanager.h"

KategoriTransaksiManager::KategoriTransaksiManager()
    : BaseManager("kategori_transaksi", false)
{
}

bool KategoriTransaksiManager::beforeCreate(QVariantMap& params)
{
    if (!params.contains("kode") || params["kode"].toString().isEmpty()) {
        params["kode"] = generateCode("kategori_transaksi", "kode", "KAT-", 3);
    }
    return true;
}

QList<QSqlRecord> KategoriTransaksiManager::getByTipe(const QString& tipe)
{
    return getWhere("tipe = :tipe COLLATE NOCASE AND is_active = 1",
                    {{ "tipe", tipe }},
                    "nama");
}

QList<QSqlRecord> KategoriTransaksiManager::getActive(const QString& orderBy, int limit)
{
    return getWhere("is_active = 1", {}, orderBy, limit);
}

QList<QSqlRecord> KategoriTransaksiManager::getChildren(int parentId)
{
    return getWhere("parent_id = :parent_id AND is_active = 1",
                    {{ "parent_id", parentId }},
                    "nama");
}

bool KategoriTransaksiManager::deactivate(int id)
{
    return update(id, {{ "is_active", 0 }});
}
