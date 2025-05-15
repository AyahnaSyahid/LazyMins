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
    void on_actionCreateOrder_triggered();
    void on_actionCreateCustomer_triggered();
    void on_actionCreateProduct_triggered();
    void on_actionCreateUser_triggered();

private:
    Database* base;
    Ui::MainWindow* ui;
    
};

#endif
