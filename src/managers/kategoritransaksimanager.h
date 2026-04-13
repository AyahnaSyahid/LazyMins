#pragma once

#include "basemanager.h"

// ============================================================================
// KategoriTransaksiManager — tabel: kategori_transaksi
// ============================================================================
class KategoriTransaksiManager : public BaseManager
{
public:
    explicit KategoriTransaksiManager()
        : BaseManager("kategori_transaksi") {}

    QList<QSqlRecord> getByTipe(const QString& tipe); // 'pemasukan' | 'pengeluaran'
    QList<QSqlRecord> getActive();
    QList<QSqlRecord> getRootCategories();
    QList<QSqlRecord> getChildren(int parentId);
    std::optional<QSqlRecord> findByNama(const QString& nama);
};
