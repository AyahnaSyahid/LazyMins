#ifndef TableManager_H
#define TableManager_H

#include <QObject>

class Database;
class QSqlRecord;
class QSqlTableModel;
class TableManager : public QObject {
    Q_OBJECT
    
public:
    TableManager(const QString& tableName, Database* = nullptr);
    ~TableManager();
    inline const QSqlTableModel* model() { return tableModel; }
    QSqlRecord record() const;

protected:
    QSqlTableModel *tableModel;
    Database *db;

};

#endif