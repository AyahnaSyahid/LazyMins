
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
    OrderManager(Database* = nullptr);
    ~OrderManager();
    QDialog* createDialog(QWidget* =nullptr);
    QDialog* editDialog(int oid, QWidget* =nullptr);
    QDialog* editDialog(const QSqlRecord& , QWidget* =nullptr);

public slots:
    void createOrder();
    void editOrder(int oid);
    void insertRecord(const QSqlRecord&);
    void updateRecord(const QSqlRecord&);

signals:
    void insertStatus(const QSqlError&, const QSqlRecord&);
    void updateStatus(const QSqlError&, const QSqlRecord&);
    void orderCreated();
    void orderModified();
    void customerCreated();
    void productCreated();

private:
    Database* db;
};

#endif