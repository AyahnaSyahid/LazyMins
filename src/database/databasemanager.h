#pragma once

#include <QSqlDatabase>
class DatabaseManager
{
  public:
    static bool initSchema(QSqlDatabase &db);
    static DatabaseManager& instance();
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(DatabaseManager &) = delete;
    bool initializeFromSetup(const QString &dbPath,
                             const QString &superUser,
                             const QString &password);
    void setDatabase(QSqlDatabase &db);
    bool isOpen() const;

    QSqlDatabase &database() { return m_database;}
    QSqlError lastError() const;
    
    bool isFirstRun() const { return m_isFirstRun; }
  private:
    DatabaseManager();
    ~DatabaseManager();
    bool verifySchema(QSqlDatabase &db);

    QSqlDatabase m_database;
    bool m_databaseReady;
    bool m_isFirstRun;
};
