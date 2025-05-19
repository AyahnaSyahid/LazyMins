#ifndef MainWindow_H
#define MainWindow_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class Database;
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
    void openPaymentFor(int);

private:
    Database* db;
    Ui::MainWindow* ui;
};

#endif
