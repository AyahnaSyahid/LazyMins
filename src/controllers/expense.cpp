#include "expense.h"

#include <QSqlQuery>

#include "src/managers/transaksimanager.h"
#include "src/managers/akuntransaksimanager.h"
#include "src/utils/sessionmanager.h"
#include "src/utils/sqltransaction.h"

bool ExpenseController::recordExpense(int akun_id, qint64 jumlah,
                                      const QString& keterangan, int* recordId,
                                      QString* error) {
  SqlTransaction tr(BaseManager::connection);
  if (!tr.started()) {
    if (error) *error = "Tidak dapat memulai transaksi database";
    return false;
  }
  TransaksiManager trm;
  QSqlQuery q(BaseManager::connection);

  q.prepare(R"-( 
    INSERT INTO transaksi (
        akun_id, 
        transaction_number, 
        admin_id, 
        kategori_id, 
        tipe, 
        deskripsi, 
        amount_before, 
        amount, 
        amount_after, 
        tanggal
    )
    SELECT 
        :akun_id, 
        :transaction_number, 
        :admin_id, 
        :kategori_id, 
        :tipe, 
        :deskripsi,
        saldo,                    -- Menjadi amount_before
        :amount,                  -- Menjadi amount
        (saldo + :amount),        -- Menjadi amount_after
        :tanggal
    FROM akun_transaksi 
    WHERE id = :akun_id 
    LIMIT 1; )-");
  q.bindValue(":akun_id", akun_id);
  q.bindValue(":transaction_number", trm.nextNumber());
  q.bindValue(":admin_id", SessionManager::instance().currentUserId());
  q.bindValue(":kategori_id", 2);
  q.bindValue(":tipe", "pengeluaran");
  q.bindValue(":deskripsi", keterangan);
  q.bindValue(":amount", - jumlah);
  q.bindValue(":tanggal",
              QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd hh:mm:ss"));
  if (!q.exec()) {
    if (error) *error = q.lastError().text();
    return false;
  }
  qDebug() << "ExpenseController Transaksi insert success";
  if (recordId) *recordId = q.lastInsertId().toInt();
  // update saldo
  
  q.prepare("UPDATE akun_transaksi SET saldo = saldo + :amount,  updated_at = CURRENT_TIMESTAMP WHERE id = :id");
  q.bindValue(":amount", - jumlah);
  q.bindValue(":id", akun_id);
  
  if (!q.exec()) {
    if (error) *error = q.lastError().text();
    return false;
  }
  qDebug() << "ExpenseController akun_transaksi update success";
  return tr.commit();
}
