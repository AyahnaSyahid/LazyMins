#include "managers.h"

// ============================================================================
// TransaksiManager
// ============================================================================

QList<QSqlRecord> TransaksiManager::getByTipe(const QString& tipe)
{
    return getWhere("tipe = :tipe", {{"tipe", tipe}}, "tanggal DESC");
}

QList<QSqlRecord> TransaksiManager::getByAdmin(int adminId)
{
    return getWhere("admin_id = :admin_id", {{"admin_id", adminId}}, "tanggal DESC");
}

QList<QSqlRecord> TransaksiManager::getByDateRange(const QDate& from, const QDate& to)
{
    return getWhere(
        "tanggal BETWEEN :from AND :to",
        {{"from", dateToSql(from)}, {"to", dateToSql(to)}},
        "tanggal DESC");
}

QList<QSqlRecord> TransaksiManager::getByKategori(int kategoriId)
{
    return getWhere("kategori_id = :kategori_id", {{"kategori_id", kategoriId}}, "tanggal DESC");
}

QList<QSqlRecord> TransaksiManager::getByReference(const QString& referenceType, int referenceId)
{
    return getWhere(
        "reference_type = :rt AND reference_id = :rid",
        {{"rt", referenceType}, {"rid", referenceId}});
}

std::optional<QSqlRecord> TransaksiManager::lastTransaction() const {
  QSqlQuery q("SELECT * FROM transaksi ORDER BY id DESC LIMIT 1", BaseManager::connection);
  if(!q.next()) {
    return std::nullopt;
  }
  return q.record();
}

qint64 TransaksiManager::sumByTipe(const QString& tipe, const QDate& from, const QDate& to)
{
    QSqlQuery q(BaseManager::connection);
    QString sql = "SELECT COALESCE(SUM(jumlah), 0) AS total FROM transaksi WHERE tipe = :tipe";
    if (from.isValid() && to.isValid())
        sql += " AND tanggal BETWEEN :from AND :to";
    q.prepare(sql);
    q.bindValue(":tipe", tipe);
    if (from.isValid() && to.isValid()) {
        q.bindValue(":from", dateToSql(from));
        q.bindValue(":to",   dateToSql(to));
    }
    if (q.exec() && q.next())
        return q.value("total").toLongLong();
    return 0;
}

QString TransaksiManager::generateTransactionNumber(const QString& prefix)
{
  auto q = baseQuery();
  q.prepare(QString("SELECT '%1-' || '%2-' || printf('%05d',COALESCE(COUNT(*), 0) + 1) AS next_val "
                    "FROM transaksi WHERE date(tanggal) = date('now')")
                .arg(prefix, QDate::currentDate().toString("yyyyMMdd")));
  if (q.exec() && q.next()) {
    return q.value("next_val").toString();
  }
  return generateCode("transaksi", "id", prefix);
}

bool TransaksiManager::beforeCreate(QVariantMap& params)
{
    if (!params.contains("transaction_number") || params["transaction_number"].toString().isEmpty())
        params["transaction_number"] = generateTransactionNumber();
    if (!params.contains("tanggal"))
        params["tanggal"] = QDate::currentDate().toString("yyyy-MM-dd");
    return true;
}

bool TransaksiManager::afterCreate(const QSqlRecord& rc) {
  auto akun_id = rc.value("akun_id").toInt();
  // Menggunakan amount_after sebagai saldo final akun
  auto tr_amount = rc.value("amount_after").toLongLong();
  auto admin_id = rc.value("admin_id").toInt(); 

  AkunTransaksiManager atm;
  // Perbaikan: variabel adminId seharusnya admin_id (sesuai definisi di atas)
  if ( !atm.updateSaldo(akun_id, tr_amount, admin_id) ) { 
    setErrorString ("Gagal mengupdate saldo akun_transaksi");
    return false;
  }
  return true;
}