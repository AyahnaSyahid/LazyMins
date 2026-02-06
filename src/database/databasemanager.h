#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
class DatabaseManager
{
  public:
    static DatabaseManager& instance();
    bool initialize(const QHash<QString, QVariant> &settings);
    
    bool isOpen() const;
    QSqlDatabase &database() { return m_database;}
    bool transaction();
    bool commit();
    bool rollback();
    
    QSqlError lastError() const;
    ~DatabaseManager();
    
  private:
    DatabaseManager();
    QSqlDatabase m_database;
    bool m_databaseReady;
    bool initSchema(QSqlDatabase &db);
    bool verifySchema(QSqlDatabase &db);
    
};

#endif