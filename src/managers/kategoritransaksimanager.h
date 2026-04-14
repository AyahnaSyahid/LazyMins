#pragma once
#include "basemanager.h"

class KategoriTransaksiManager : public BaseManager
{
public:
    explicit KategoriTransaksiManager();

    QList<QSqlRecord> getByTipe(const QString& tipe); // 'pemasukan' | 'pengeluaran'
    QList<QSqlRecord> getActive(const QString& orderBy = "nama", int limit = -1);
    QList<QSqlRecord> getChildren(int parentId);
    bool deactivate(int id);

protected:
    bool beforeCreate(QVariantMap& params) override;
};
