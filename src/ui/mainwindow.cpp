#include "mainwindow.h"
#include "files/ui_mainwindow.h"
#include "database.h"
#include "usermanager.h"
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
#include "loginform.h"
#include "permissiondialog.h"
#include "passworddialogs.h"

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
    
    UserManager *uman = db->findChild<UserManager*>("userManager");
    connect(uman, UserManager::userLoggedIn, this, MainWindow::onUserLoggedIn);
    connect(uman, UserManager::userLoggedOut, this, MainWindow::onUserLoggedOut);
    connect(ui->actionLogOut, QAction::triggered, uman, UserManager::logout);
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

void MainWindow::onUserLoggedIn(int uid) {
  auto uman = db->findChild<UserManager*>("userManager");
  if(!uman) return;
  auto actions = findChildren<QAction*>();
  if(uman->hasPermission(uid, "GRANT_EVERYTHING")) {
    for(auto a = actions.cbegin(); a != actions.cend(); ++a) {
      (*a)->setEnabled(true);
      (*a)->setToolTip("");
    }
    return;
  }
  QAction *aptr;
  for(auto a = actions.cbegin(); a != actions.cend(); ++a) {
    aptr = (*a);
    QVariant perm_var = aptr->property("requiredPerm");
    if(perm_var.isValid()) {
      if(uman->hasPermission(uid, perm_var.toString())) {
        aptr->setEnabled(true);
        aptr->setToolTip("");
      } else {
        aptr->setEnabled(false);
        aptr->setToolTip("Akses dibatasi");
      }
    } else {
    aptr->setEnabled(true);
    aptr->setToolTip("");
    }
  }
}

void MainWindow::onUserLoggedOut() {
  LoginForm *lform = new LoginForm(db->findChild<UserManager*>("userManager"), this);
  lform->setAttribute(Qt::WA_DeleteOnClose);
  hide();
  connect(lform, &QDialog::accepted, this, &MainWindow::show);
  connect(lform, &QDialog::rejected, this, &MainWindow::close);
  // connect(lform, &QDialog::destroyed, [](){qDebug() << "LoginForm deleted"; });
  lform->setWindowTitle("Masuk lagi");
  lform->open();
}

void MainWindow::on_actionPerizinan_triggered() {
  auto uman = db->findChild<UserManager*>("userManager");
  auto pd = new PermissionDialog(uman->currentUser(), this);
  pd->setAttribute(Qt::WA_DeleteOnClose);
  pd->open();
}

void MainWindow::on_actionPasswordSaya_triggered() {
  auto cpd = new ChangePasswordDialog(db->findChild<UserManager*>("userManager"), this);
  cpd->setAttribute(Qt::WA_DeleteOnClose);
  cpd->open();
}

void MainWindow::on_actionPasswordUserLain_triggered(){
  // select user
  UserSelectorDialog usd(this);
  if(usd.exec() == QDialog::Accepted) {
    auto copd = new ChangeOtherPasswordDialog(usd.selectedId(), db->findChild<UserManager*>("userManager"), this);
    
  }
}
