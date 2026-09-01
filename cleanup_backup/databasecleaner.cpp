#include "databasecleaner.h"
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QStringList>
#include <QDebug>

bool DatabaseCleaner::clearAllDataAndResetTriggers(const QString& connectionName = QSqlDatabase::defaultConnection) {
  QSqlDatabase db = QSqlDatabase::database(connectionName);

  if (!db.isOpen()) {
      qCritical() << "Database tidak terbuka!";
      return false;
  }

  // 1. Mulai Transaksi untuk keamanan data
  if (!db.transaction()) {
      qCritical() << "Gagal memulai transaksi:" << db.lastError().text();
      return false;
  }

  try {
      // 2. Nonaktifkan Foreign Key Checks (Opsional, tergantung DB)
      // SQLite: "PRAGMA foreign_keys = OFF;"
      // MySQL: "SET FOREIGN_KEY_CHECKS = 0;"
      QSqlQuery query(db);
      query.exec("PRAGMA foreign_keys = OFF;"); 

      // 3. Ambil daftar semua tabel
      QStringList tables = db.tables();
      
      // 4. Proses Triggers: Simpan definisi dan hapus
      // Catatan: Logika pengambilan metadata trigger berbeda tiap DB. 
      // Contoh di bawah menggunakan logika umum.
      struct TriggerInfo { QString name; QString table; QString statement; };
      QList<TriggerInfo> savedTriggers;

      // Ambil definisi trigger (Contoh untuk SQLite)
      query.exec("SELECT name, tbl_name, sql FROM sqlite_master WHERE type='trigger'");
      while (query.next()) {
          savedTriggers.append({
              query.value(0).toString(),
              query.value(1).toString(),
              query.value(2).toString()
          });
      }

      // Hapus Triggers lama
      for (const auto& trig : savedTriggers) {
          query.exec(QString("DROP TRIGGER IF EXISTS %1").arg(trig.name));
      }

      // 5. Hapus Data di setiap Tabel
      for (const QString& tableName : tables) {
          if (tableName.startsWith("sqlite_")) continue; // Lewati tabel sistem
          
          if (!query.exec(QString("DELETE FROM %1").arg(tableName))) {
              throw std::runtime_error(query.lastError().text().toStdString());
          }
          
          // Reset Auto Increment (Optional)
          query.exec(QString("DELETE FROM sqlite_sequence WHERE name='%1'").arg(tableName));
      }

      // 6. Restrukturisasi: Buat ulang Triggers
      for (const auto& trig : savedTriggers) {
          if (!query.exec(trig.statement)) {
              qWarning() << "Gagal membuat ulang trigger" << trig.name << ":" << query.lastError().text();
          }
      }

      // 7. Aktifkan kembali Foreign Key dan Commit
      query.exec("PRAGMA foreign_keys = ON;");
      
      if (db.commit()) {
          qInfo() << "Berhasil mengosongkan database dan merestrukturisasi triggers.";
          return true;
      }

  } catch (const std::exception& e) {
      qCritical() << "Error terjadi:" << e.what();
      db.rollback();
  }

  return false;
}