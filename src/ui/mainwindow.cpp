#include "mainwindow.h"
#include "files/ui_mainwindow.h"
#include "database.h"
#include "createorderdialog.h"
#include "createproductdialog.h"
#include "createcustomerdialog.h"
#include "createpaymentdialog.h"
#include "invoicesmanagerdialog.h"
#include "createaccountdialog.h"
#include "dockwidgets/dailywidget.h"
#include "dockwidgets/customerorderswidget.h"
#include "createinvoicedialog.h"
#include "editorderdialog.h"

#include <QSqlQuery>
#include <QItemSelectionModel>
#include <QMessageBox>

#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QDockWidget>
#include <QtDebug>

#define REGIST_DIALOGS(DCL, DNM, MWIN) \
    if(!_dialogs.contains(#DNM)) {\
        DCL* cod = new DCL(db);\
        cod->setObjectName(#DNM);\
        cod->setAttribute(Qt::WA_DeleteOnClose);\
        connect(cod, &QObject::destroyed, [this, name = cod->objectName()]() {\
            this->dialogDestroyed(name);\
        });\
        _dialogs.insert(#DNM, cod);\
        cod->show();\
    } else {\
        _dialogs.value(#DNM)->activateWindow();\
    }\

MainWindow::MainWindow(Database* _d, QWidget* parent)
 : ui(new Ui::MainWindow), db(_d), QMainWindow(parent)
{
    ui->setupUi(this);
    addDockWidget(Qt::LeftDockWidgetArea, new DailyDockWidget(db, this));
    addDockWidget(Qt::LeftDockWidgetArea, new CustomerOrdersDockWidget(db, this));
    
    connect(db, SIGNAL(paymentRequest(int)), this, SLOT(openPaymentFor(int)));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_actionOrdersManager_triggered() {
    REGIST_DIALOGS(CreateOrderDialog, createOrderDialog, this);
}

void MainWindow::on_actionAddCustomer_triggered() {
    REGIST_DIALOGS(CreateCustomerDialog, createCustomerDialog, this);
}

void MainWindow::on_actionAddProduct_triggered() {
    REGIST_DIALOGS(CreateProductDialog, createProductDialog, this);
}

void MainWindow::on_actionInvoicesManager_triggered() {
    REGIST_DIALOGS(InvoicesManagerDialog, invoicesManagerDialog, this);
}

void MainWindow::on_actionAddUser_triggered() {
    CreateAccountDialog* cad = new CreateAccountDialog(db, this);
    cad->setAttribute(Qt::WA_DeleteOnClose);
    cad->open();
}

void MainWindow::openPaymentFor(int inv) {
    CreatePaymentDialog* cpd = new CreatePaymentDialog(inv, db);
    cpd->setAttribute(Qt::WA_DeleteOnClose);
    cpd->show();
}

void MainWindow::dialogDestroyed(const QString& name) {
    _dialogs.remove(name);
}

void MainWindow::createInvoiceForOrdersReceiver(const QList<int>& orders) {
    QStringList sOrders;
    for(auto oo = orders.cbegin(); oo != orders.cend(); ++oo) {
        sOrders << QString::number(*oo);
    }
    auto cid = CreateInvoiceDialog::fromOrderList(orders, db, this);
    cid->setAttribute(Qt::WA_DeleteOnClose);
    cid->setWindowModality(Qt::WindowModal);
    emit createInvoiceForOrdersReceived();
    connect(cid, SIGNAL(openPayment(int)), this, SLOT(openPaymentFor(int)));
    cid->open();
}

void MainWindow::openOrderEditor(int o) {
    QSqlQuery q;
    q.prepare("SELECT * FROM orders WHERE order_id = ?");
    q.addBindValue(o);
    q.exec();
    if(! q.next()) {
        QMessageBox::information(this, "Kesalahan", tr("tidak ditemukan order dengan id %1").arg(o));
        return;
    }
    EditOrderDialog* eo = new EditOrderDialog(q.record(), db, this);
    eo->setAttribute(Qt::WA_DeleteOnClose);
    eo->open();
}