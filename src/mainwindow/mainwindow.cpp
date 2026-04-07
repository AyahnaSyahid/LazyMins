#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "src/dialogs/konsumendialog.h"
#include "src/dialogs/userdialog.h"
#include "src/dialogs/orderdialog.h"
#include "src/dialogs/productdialog.h"
#include "src/dialogs/instantorderdialog.h"
#include "src/utils/sessionmanager.h"
#include "src/display/produkdataviewer.h"
#include "src/display/finishingservicesviewer.h"
#include "src/display/orderdataviewer.h"
#include "src/display/invoicedataviewer.h"
#include "src/dialogs/logindialog.h"
#include "src/dialogs/kategoriprodukdialog.h"
#include "src/managers/adminmanager.h"
#include <QDockWidget>
#include <QMessageBox>

namespace {
  void connectCreateActionToFormDialog(QAction *action, 
                                      const QString& formTitle, 
                                      std::function<FormDialog*()> formOpener, 
                                      QObject *me = nullptr) {
    me->connect(action, &QAction::triggered, [me, formTitle, formOpener](){
      auto dd = formOpener();
      dd->prepareCreate();
      dd->setWindowTitle(formTitle);
      dd->setAttribute(Qt::WA_DeleteOnClose);
      dd->open();
    });
  }
  void connectModifyActionToFormDialog(QAction *action, 
                                      const QString& formTitle, 
                                      std::function<FormDialog*()> formOpener, 
                                      std::function<QSqlRecord()> recordProvider, 
                                      QObject *me=nullptr) {
    me->connect(action, &QAction::triggered, [me, formTitle, formOpener, recordProvider](){
      auto dd = formOpener();
      dd->prepareModify(recordProvider());
      dd->setWindowTitle(formTitle);
      dd->setAttribute(Qt::WA_DeleteOnClose);
      dd->open();
    });
  }
}

MainWindow::MainWindow(QWidget *p) :
ui(new Ui::MainWindow), QMainWindow(p) {
  ui->setupUi(this);
  ui->menuToolbar->addAction(ui->addDataToolbar->toggleViewAction());
  ui->menuToolbar->addAction(ui->transactionToolbar->toggleViewAction());
  auto dockSetup = [](QDockWidget *dw, const QString &title, QWidget *widget) -> QDockWidget* { dw->setWidget(widget); dw->setWindowTitle(title); return dw; };
  
  auto dv1 = new ProdukDataViewer;
  auto dsP = dockSetup(new QDockWidget(this), "Data Produk", dv1);
  addDockWidget(Qt::TopDockWidgetArea, dsP);
  dv1->setPageSize(100);
  dv1->refresh();
  ui->menuView->addAction(dsP->toggleViewAction());
  connect(ui->actionProdukAdd, &QAction::triggered, dv1->addProductAction(), &QAction::trigger);

  auto fs1 = new FinishingServicesViewer;
  auto dsF = dockSetup(new QDockWidget(this), "Data Finishing", fs1);
  addDockWidget(Qt::LeftDockWidgetArea, dsF);
  fs1->setPageSize(100);
  fs1->refresh();
  ui->menuView->addAction(dsF->toggleViewAction());

  tabifyDockWidget(dsP, dsF);

  auto ord1 = new OrderDataViewer;
  auto dsO = dockSetup(new QDockWidget(this), "Data Orders", ord1);
  addDockWidget(Qt::TopDockWidgetArea, dsO);
  ord1->setPageSize(50);
  ord1->refresh();
  ui->menuView->addAction(dsO->toggleViewAction());
  
  auto idv = new InvoiceDataViewer;
  auto dsI = dockSetup(new QDockWidget(this), "Data Invoice", idv);
  addDockWidget(Qt::TopDockWidgetArea, dsI);
  idv->setPageSize(50);
  idv->refresh();
  ui->menuView->addAction(dsI->toggleViewAction());
  
  tabifyDockWidget(dsO, dsI);
  
  connectCreateActionToFormDialog(ui->actionKonsumenAdd, "Tambah data konsumen baru", [this](){ return new KonsumenDialog(this); }, this);
  // connectCreateActionToFormDialog(ui->actionProdukAdd, "Tambah data produk baru", [this, dv1]()
    // { auto pd =  new ProductDialog(this);
      // pd->connect(pd, &QDialog::accepted, dv1, &DataViewer::refresh);
      // return pd;
    // }, this);
  
  auto createOrderDialog = [this, ord1, dv1]() {
    auto d = new OrderDialog(this);
    d->setAttribute(Qt::WA_DeleteOnClose);
    connect(d, &OrderDialog::accepted, ord1, &OrderDataViewer::refresh);
    connect(d, &OrderDialog::accepted, dv1, &DataViewer::refresh);
    d->open();
  };
  
  connect(ui->actionOrderCreate, &QAction::triggered, createOrderDialog);
  connectCreateActionToFormDialog(ui->actionAdminAdd, "Tambah data admin baru", 
    [this](){ 
      auto ud = new UserDialog(this);
      return ud; }, this);
  connect(ui->actionInstantOrderCreate, &QAction::triggered, [this](){
    auto dialog = new InstantOrderDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    // connect signal
    // connect(dialog, &QDialog::accepted, ) 
    dialog->open();
  });
  
  connect(ui->actionAddKatProduk, &QAction::triggered, [this]() {
    auto dia = new KategoriProdukDialog(this);
    dia->setAttribute(Qt::WA_DeleteOnClose);
    dia->prepareCreate();
    dia->open();
  });
  
  // UserSession
  auto &sm = SessionManager::instance();
  connect(&sm, &SessionManager::loginSuccess, this, &MainWindow::currentUserChanged);
  connect(&sm, &SessionManager::userLogout, this, &MainWindow::openLoginForm);
  connect(ui->actionKeluar, &QAction::triggered, &sm, &SessionManager::logout);
}

MainWindow::~MainWindow() {delete ui;}

void MainWindow::setupToolbarActions() {
  // currently no dynamic action setup is needed, but this function can be used in the future if we want to enable/disable actions based on user role or other conditions
}

void MainWindow::openLoginForm() {
  auto cu = SessionManager::instance().currentUser();
  if (cu.has_value()) {
    return ;
  }
  hide();
  auto ld = new LoginDialog();
  connect(ld, &LoginDialog::accepted, this, &QWidget::show);
  ld->setAttribute(Qt::WA_DeleteOnClose);
  ld->open();
}

void MainWindow::currentUserChanged() {
  auto &sm = SessionManager::instance();
  auto opt_user = sm.currentUser();
  if (!opt_user) {
    QMessageBox::critical(this, "Fatal Error", "Tidak dapat mendeteksi user valid !!\nApplikasi akan di terminasi");
    qApp->quit();
    return ;
  }
  auto rec_user = *opt_user;
  AdminManager a_man;
  // lakukan preparasi ui untuk current user dan pembatasan akses GUI
  auto has_super_user = a_man.userHasRole(rec_user.value("id").toInt(), "super_admin");
  // qDebug() << QString("%1 : %2").arg(rec_user.value("username").toString()).arg(has_super_user);
  if (!has_super_user) {
    ui->actionAdminAdd->setEnabled(false);
  } else {
    ui->actionAdminAdd->setEnabled(true);
  }
}