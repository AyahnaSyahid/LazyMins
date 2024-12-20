#ifndef OrdersModuleH
#define OrdersModuleH

#include <QObject>

class QMainWindow;
class QAction;
class OrdersModule : public QObject {
    Q_OBJECT

public:
    OrdersModule(QObject* =nullptr);
    void installMenu(QMainWindow* mw=nullptr);
    void installDocker(QMainWindow* mw=nullptr);

public slots:
    void onAddOrder();
    void onCreatePayment();
    void onModifyOrder();

signals:
    void orderAdded();
    void orderModified();
    void orderRemoved();

private:
    QMainWindow* mainWindow;
};

#endif // OrdersModuleH