#pragma once
#include "basemanager.h"

// TransaksiManager bersifat IMMUTABLE (append-only / buku besar).
// update() dan remove() diblokir — transaksi tidak boleh diubah atau dihapus.
class TransaksiManager : public BaseManager
{
public:
    static QString nextNumber();
    explicit TransaksiManager();

    QList<QSqlRecord> getByAkun(int akunId, const QString& orderBy = "tanggal DESC");
    QList<QSqlRecord> getByTipe(const QString& tipe, const QString& orderBy = "tanggal DESC");
    QList<QSqlRecord> getByReference(const QString& referenceType, int referenceId);
    QList<QSqlRecord> getByDateRange(const QDate& from, const QDate& to, int akunId = -1);

    // DIBLOKIR — transaksi bersifat immutable
    bool update(int id, const QVariantMap& params) override;
    bool remove(int id) override;

protected:
    bool beforeCreate(QVariantMap& params) override;
};
