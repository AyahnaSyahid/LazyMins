#ifndef TableManager_H
#define TableManager_H

#include <QObject>

class Database;
class QSqlRecord;
class QSqlTableModel;

class TableManager : public QObject {
    Q_OBJECT
    
public:
    TableManager(const QString&, Database*, QObject* parent=nullptr);
    ~TableManager();
    const QSqlTableModel* model();
    QSqlRecord record() const;

protected:
    Database *db;
    QSqlTableModel* _model;
};

#endif