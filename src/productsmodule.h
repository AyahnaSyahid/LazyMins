#ifndef ProductsModuleH
#define ProductsModuleH

#include <QObject>

class QMainWindow;
class ProductsModule : public QObject {
    Q_OBJECT

public:
    ProductsModule(QObject *mw);
    void installMenu(QMainWindow* =nullptr);
    void installDocker(QMainWindow* =nullptr);

public slots:
    void onAddProduct();
    void onRemoveProduct();
    void onEditProduct();

signals:
    void productModified();
    void productAdded();
    void productRemoved();

private:
    QMainWindow *mainWindow;
};

#endif // ProductsModuleH