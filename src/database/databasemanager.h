#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
class DatabaseManager
{
  public:
    static DatabaseManager& instance();
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(DatabaseManager &) = delete;
    bool initialize(const QHash<QString, QVariant> &settings);
    
    bool isOpen() const;
    QSqlDatabase &database() { return m_database;}
    bool transaction();
    bool commit();
    bool rollback();
    
    QSqlError lastError() const;
    
  private:
    DatabaseManager();
    ~DatabaseManager();
    QSqlDatabase m_database;
    bool m_databaseReady;
    bool initSchema(QSqlDatabase &db);
    bool verifySchema(QSqlDatabase &db);
};

#endif