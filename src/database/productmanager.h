#ifndef ProductManager_H
#define ProductManager_H

#include "tablemanager.h"

class QDialog;
class ProductManager : public TableManager {
    Q_OBJECT

public:
    ProductManager(QObject* = nullptr);
    ~ProductManager();
    
    QDialog* createDialog(QWidget* =nullptr);
    QDialog* editDialog(int, QWidget* =nullptr);

public slots:
    void createProduct();
    void editProduct(int);

signals:
    void created();
    void modified();
};


#endif // End ProductManager_H