#pragma once

#include <QSqlDatabase>
class DatabaseManager {
 public:
  static bool initSchema(QSqlDatabase& db);
  static DatabaseManager& instance();
  DatabaseManager(const DatabaseManager&) = delete;
  DatabaseManager& operator=(DatabaseManager&) = delete;
  bool initializeFromSetup(const QString& dbPath, const QString& superUser,
                           const QString& password);
  void setDatabase(QSqlDatabase& db);
  bool isOpen() const;

  QSqlDatabase& database() { return m_database; }
  QSqlError lastError() const;

  bool isFirstRun() const { return m_isFirstRun; }
  bool migrate();

 private:
  DatabaseManager();
  ~DatabaseManager();
  bool verifySchema(QSqlDatabase& db);

  QSqlDatabase m_database;

  // m_databaseReady HANYA menunjukkan bahwa database sudah bisa dibuka
  bool m_databaseReady;

  // m_isFirstRun HANYA menunjukkan bahwa file settings (LAdmins.ini) belum ada
  bool m_isFirstRun;
};
