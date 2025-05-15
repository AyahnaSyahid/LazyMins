
#ifndef OrderManager_H
#define OrderManager_H

#include "tablemanager.h"

class QSqlRecord;
class QSqlError;
class QDialog;
class Database;
class OrderManager : public TableManager
{
    Q_OBJECT
public:
    OrderManager(Database* , QObject* parent=nullptr);
    ~OrderManager();

public slots:
    void insertRecord(const QSqlRecord&);
    void updateRecord(const QSqlRecord&);

signals:
    void insertStatus(const QSqlError&, const QSqlRecord&);
    void updateStatus(const QSqlError&, const QSqlRecord&);

private:
    Database* db;
};

#endif