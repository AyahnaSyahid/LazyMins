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
    QDialog* editDialog(QWidget* =nullptr);
};

#endif