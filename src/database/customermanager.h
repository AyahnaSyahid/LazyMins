#ifndef CustomerManager_H
#define CustomerManager_H

#include "tablemanager.h"

class QDialog;
class CustomerManager : public TableManager {
    Q_OBJECT

public:
    CustomerManager(QObject* =nullptr);
    ~CustomerManager();
    
    QDialog* createDialog(QWidget* =nullptr);
    QDialog* editDialog(int cid, QWidget* =nullptr);

public slots:
    void createCustomer();
    void editCustomer(int a);

signals:
    void created();
    void modified();

};

#endif