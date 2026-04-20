#include "transaksimanager.h"

QString TransaksiManager::nextNumber() {
  return generateCode("transaksi", "transaction_number", "TRX-", 5, true);
}

TransaksiManager::TransaksiManager()
    : BaseManager("transaksi", false)
{
}

bool TransaksiManager::beforeCreate(QVariantMap& params)
{
    if (!params.contains("transaction_number") || params["transaction_number"].toString().isEmpty()) {
        params["transaction_number"] = nextNumber();
    }
    return true;
}

// DIBLOKIR
bool TransaksiManager::update(int id, const QVariantMap& params)
{
    Q_UNUSED(id)
    Q_UNUSED(params)
    setErrorString("Transaksi bersifat immutable dan tidak dapat diubah.");
    return false;
}

// DIBLOKIR
bool TransaksiManager::remove(int id)
{
    Q_UNUSED(id)
    setErrorString("Transaksi bersifat immutable dan tidak dapat dihapus.");
    return false;
}

QList<QSqlRecord> TransaksiManager::getByAkun(int akunId, const QString& orderBy)
{
    return getWhere("akun_id = :akun_id",
                    {{ ":akun_id", akunId }},
                    orderBy);
}

QList<QSqlRecord> TransaksiManager::getByTipe(const QString& tipe, const QString& orderBy)
{
    return getWhere("tipe = :tipe COLLATE NOCASE",
                    {{ ":tipe", tipe }},
                    orderBy);
}

QList<QSqlRecord> TransaksiManager::getByReference(const QString& referenceType, int referenceId)
{
    return getWhere(
        "reference_type = :ref_type AND reference_id = :ref_id",
        {{ ":ref_type", referenceType }, { ":ref_id", referenceId }},
        "tanggal DESC"
    );
}

QList<QSqlRecord> TransaksiManager::getByDateRange(const QDate& from, const QDate& to, int akunId)
{
    QString condition = "tanggal >= :from AND tanggal <= :to";
    QVariantMap bindings = {
        { ":from", from.toString(Qt::ISODate) },
        { ":to",   to.toString(Qt::ISODate) }
    };
    if (akunId > 0) {
        condition += " AND akun_id = :akun_id";
        bindings[":akun_id"] = akunId;
    }
    return getWhere(condition, bindings, "tanggal DESC");
}
