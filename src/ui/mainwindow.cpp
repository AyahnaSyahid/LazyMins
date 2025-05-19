#include "mainwindow.h"
#include "files/ui_mainwindow.h"
#include "database.h"
#include "createorderdialog.h"
#include "createproductdialog.h"
#include "createcustomerdialog.h"
#include "createpaymentdialog.h"
#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QtDebug>

MainWindow::MainWindow(Database* _d, QWidget* parent)
 : ui(new Ui::MainWindow), db(_d), QMainWindow(parent)
{
    ui->setupUi(this);
    connect(db, SIGNAL(paymentRequest(int)), this, SLOT(openPaymentFor(int)));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_actionAddOrder_triggered() {
    CreateOrderDialog* cod = new CreateOrderDialog(db, this);
    cod->setAttribute(Qt::WA_DeleteOnClose);
    cod->open();
}

void MainWindow::on_actionAddCustomer_triggered() {
    CreateCustomerDialog* ccd = new CreateCustomerDialog(db, this);
    ccd->setAttribute(Qt::WA_DeleteOnClose);
    ccd->open();
}

void MainWindow::on_actionAddProduct_triggered() {
    CreateProductDialog* cpd = new CreateProductDialog(db, this);
    cpd->setAttribute(Qt::WA_DeleteOnClose);
    cpd->open();
}

void MainWindow::on_actionAddUser_triggered() {
    
}

void MainWindow::openPaymentFor(int inv) {
    // qDebug() << "Open Paymen Triggered";
    CreatePaymentDialog* cpd = new CreatePaymentDialog(inv, db);
    cpd->setAttribute(Qt::WA_DeleteOnClose);
    cpd->open();
}