#ifndef CustomerModuleH
#define CustomerModuleH

class QMainWindow;
class CustomerModule : public QObject {
    Q_OBJECT
public:
    CustomerModule(QObject* = nullptr);
    void installMenu(QMainWindow* mw=nullptr);
    void installDocker(QMainWindow* mw=nullptr);

private slots:
    void onAddCustomer();
    void onManageCustomer();

signals:
    void customerAdded();
    void customerRemoved();

private:
    QMainWindow* mainWindow;
};

#endif