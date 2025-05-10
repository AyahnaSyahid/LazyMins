
#ifndef OrderManager_H
#define OrderManager_H

#include "tablemanager.h"

class QSqlRecord;
class QSqlError;
class QDialog;
class OrderManager : public TableManager
{
    Q_OBJECT

public:
    OrderManager(QObject* = nullptr);
    ~OrderManager();

    QDialog* createDialog(QWidget* =nullptr);
    QDialog* editDialog(int oid, QWidget* =nullptr);

public slots:
    void createOrder();
    void editOrder(int oid);
    void insertRecord(const QSqlRecord&);

signals:
    void insertStatus(const QSqlError&, const QSqlRecord&);
    void created();
    void modified();
};

#endif