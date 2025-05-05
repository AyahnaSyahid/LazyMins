#ifndef OrderManager_H
#define OrderManager_H

#include "tablemanager.h"

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

signals:
    void created();
    void modified();
};

#endif