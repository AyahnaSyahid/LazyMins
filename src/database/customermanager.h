#ifndef CustomerManager_H
#define CustomerManager_H

#include "tablemanager.h"

class QDialog;
class CustomersManager : public TableManager {
    Q_OBJECT

public:
    CustomersManager(QObject *parent);
    ~CustomersManager();
    
    QDialog* createDialog(QWidget* =nullptr);
    QDialog* editDialog(QWidget* =nullptr);

};

#endif