#ifndef MainWindow_H
#define MainWindow_H

#include <QMainWindow>
#include <QMap>

namespace Ui {
    class MainWindow;
}

class Database;
class QDialog;
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(Database*, QWidget* =nullptr);
    ~MainWindow();
    
private slots:
    void on_actionAddOrder_triggered();
    void on_actionAddCustomer_triggered();
    void on_actionAddProduct_triggered();
    void on_actionAddUser_triggered();
    void on_actionInvoicesManager_triggered();
    void openPaymentFor(int);
    void dialogDestroyed(const QString&);


private:
    Database* db;
    Ui::MainWindow* ui;
    QMap<QString, QDialog*> _dialogs;

};

#endif
