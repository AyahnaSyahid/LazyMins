#include "mainwindow.h"
#include "files/ui_mainwindow.h"
#include "database.h"
#include "createcustomerdialog.h"
#include "createorderdialog.h"
#include "createproductdialog.h"
#include <QMenu>
#include <QAction>
#include <QMenuBar>

MainWindow::MainWindow(Database* db, QWidget* parent)
 : ui(new Ui::MainWindow), base(db), QMainWindow(parent)
{
    ui->setupUi(this);
    
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_actionAddOrder_triggered() {
}

void MainWindow::on_actionAddCustomer_triggered() {
    CreateCustomerDialog* cc = new CreateCustomerDialog()
}

void MainWindow::on_actionAddProduct_triggered();
void MainWindow::on_actionAddUser_triggered();