#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "actiongroup.h"
#include "src/dialogs/konsumendialog.h"
#include "src/dialogs/userdialog.h"
#include "src/dialogs/edituserdialog.h"
#include "src/dialogs/orderdialog.h"
#include "src/dialogs/productdialog.h"
#include "src/dialogs/instantorderdialog.h"
#include "src/dialogs/kategoriprodukdialog.h"
#include "src/dialogs/logindialog.h"

#include "src/utils/sessionmanager.h"

#include "src/display/produkdataviewer.h"
#include "src/display/finishingservicesviewer.h"
#include "src/display/orderdataviewer.h"
#include "src/display/invoicedataviewer.h"
#include "src/display/akuntransaksidataviewer.h"
#include "src/display/konsumendataviewer.h"
#include "src/display/paymentsdataviewer.h"
#include "src/dialogs/configureserialposdialog.h"

#include "src/managers/adminmanager.h"
#include "src/managers/appsettingsmanager.h"
#include "src/printer/printservice.h"

#include "src/modul_laporan/reportview.h"
#include "src/modul_laporan/reportloader.h"

#include <QDockWidget>
#include <QMessageBox>
#include <QDate>
#include <QTimer>

#include "src/utils/posprintertestdialog.h"

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
  connect(ui->actionAddKatProduk, &QAction::triggered, dv1->addCategoryProductAction(), &QAction::trigger);

  auto fs1 = new FinishingServicesViewer;
  auto dsF = dockSetup(new QDockWidget(this), "Data Finishing", fs1);
  addDockWidget(Qt::LeftDockWidgetArea, dsF);
  fs1->setPageSize(100);
  fs1->refresh();
  ui->menuView->addAction(dsF->toggleViewAction());

  auto ord1 = new OrderDataViewer;
  auto dsO = dockSetup(new QDockWidget(this), "Data Orders", ord1);
  addDockWidget(Qt::TopDockWidgetArea, dsO);
  ord1->setPageSize(50);
  ord1->refresh();
  ui->menuView->addAction(dsO->toggleViewAction());
  connect(ui->actionOrderCreate, &QAction::triggered, ord1, &OrderDataViewer::openCreateOrderDialog);
  connect(ord1, &OrderDataViewer::orderCreated, dv1, &DataViewer::refresh);

  auto idv = new InvoiceDataViewer;
  auto dsI = dockSetup(new QDockWidget(this), "Data Invoice", idv);
  addDockWidget(Qt::TopDockWidgetArea, dsI);
  idv->setPageSize(50);
  idv->refresh();
  ui->menuView->addAction(dsI->toggleViewAction());
  connect(ui->actionInvoiceCreate, &QAction::triggered, idv, &InvoiceDataViewer::onCreateInvoice);
  connect(idv, &DataViewer::refreshed, ord1, &DataViewer::refresh); // Hati2 jangan sampai circular
  
  auto atdv = new AkunTransaksiDataViewer;
  auto dsAT = dockSetup(new QDockWidget(this), "Akun Transaksi", atdv);
  addDockWidget(Qt::BottomDockWidgetArea, dsAT);
  atdv->setPageSize(100);
  atdv->refresh();
  ui->menuView->addAction(dsAT->toggleViewAction());

  auto kdv = new KonsumenDataViewer;
  auto dsK = dockSetup(new QDockWidget(this), "Data Konsumen", kdv);
  addDockWidget(Qt::BottomDockWidgetArea, dsK);
  kdv->setPageSize(100);
  kdv->refresh();
  ui->menuView->addAction(dsK->toggleViewAction());
  connect(ui->actionKonsumenAdd, &QAction::triggered, kdv, &KonsumenDataViewer::openCreateKonsumenDialog);
  
  auto pdv = new PaymentsDataViewer;
  auto dsPy = dockSetup(new QDockWidget(this), "Pembayaran", pdv);
  addDockWidget(Qt::RightDockWidgetArea, dsPy);
  pdv->setPageSize(100);
  pdv->refresh();
  ui->menuView->addAction(dsPy->toggleViewAction());
  
  // InvoiceDataViewer bisa membuat pembayaran
  connect(idv, &InvoiceDataViewer::paymentCreated, pdv, &DataViewer::refresh);
  
  // PaymentsDataViewer bisa memverifikasi pembayaran
  connect(pdv, &PaymentsDataViewer::paymentVerified, idv, &DataViewer::refresh);

  tabifyDockWidget(dsP, dsF); // products, finishings
  dsP->raise();
  
  tabifyDockWidget(dsO, dsI); // orders, invoices
  tabifyDockWidget(dsI, dsPy); // invoices, payments
  dsO->raise();
  
  tabifyDockWidget(dsK, dsAT); // AkunTransaksi, konsumen
  dsK->raise();
  
  connectCreateActionToFormDialog(ui->actionAdminAdd, "Tambah data admin baru", 
    [this](){ 
      auto ud = new UserDialog(this);
      return ud; }, this);
  connect(ui->actionInstantOrderCreate, &QAction::triggered, [this, dv1, idv](){
    auto dialog = new InstantOrderDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &QDialog::accepted, dv1, &DataViewer::refresh); 
    connect(dialog, &QDialog::accepted, idv, &DataViewer::refresh); 
    dialog->open();
  });

  // Various actions
  auto actionGroup = new ActionGroup(this);
  actionGroup->setRootWidget(this);
  ui->menuTambah->addSeparator();
  ui->menuTambah->addAction(actionGroup->buatAkunTransaksiAction);
  ui->menuTambah->addAction(actionGroup->catatPengeluaranAction);

  // UserSession
  auto &sm = SessionManager::instance();
  connect(&sm, &SessionManager::loginSuccess, this, &MainWindow::currentUserChanged);
  connect(&sm, &SessionManager::userLogout, this, &MainWindow::hide);
  connect(&sm, &SessionManager::userLogout, this, &MainWindow::openLoginForm);
  connect(&sm, &SessionManager::loginFailed, [this](const QString& m) {
    QMessageBox::warning(this, "Peringatan", m);
  });

  connect(ui->actionKeluar, &QAction::triggered, &sm, &SessionManager::logout);
  auto actEditAkun = new QAction("Edit info", this);
  actEditAkun->setObjectName("editAkunAction");
  ui->menuAkun->insertAction(ui->actionKeluar, actEditAkun);
  connect(actEditAkun, &QAction::triggered, [this, &sm]() {
    if (sm.currentUser()->isEmpty()) {
      QMessageBox::warning(this, "Kesalahan", "User tidak valid");
      return ;
    }
    auto ud = new EditUserDialog("", this);
    if (!ud->setUser(sm.currentUser()->value("username").toString()) || !ud->userLoaded()) {
      QMessageBox::warning( this, "Peringatan", "Username tidak ditemukan dalam database");
      ud->deleteLater();
      return ;
    }
    AdminManager aa;
    if (!aa.userHasRole(sm.currentUser()->value("id").toInt(), "super_admin")) ud->setEditRoleDisabled();
    
    ud->setAttribute(Qt::WA_DeleteOnClose);
    ud->open();
  });

  // Window Title
  AppSettingsManager apm;
  setWindowTitle(apm.getSettings("company_name").value("setting_value").toString() + "- LazyAdmins");
  
  // Printer Stuff
  ui->actionPrinterTest_2->setEnabled(false);
  connect(ui->actionPrinterTest_2, &QAction::triggered, [this](){
    PosPrinterTestDialog *pp = new PosPrinterTestDialog(this);
    pp->open();
  });
  connect(ui->actionEscPosConfig, &QAction::triggered, [this](){
    auto *pp = new ConfigureSerialPosDialog(this);
    pp->exec();
  });

  auto &p_svc = PrintService::instance();
  connect(idv, &InvoiceDataViewer::paymentCreated, &p_svc, &PrintService::onPaymentCreated); 
  connect(idv, &InvoiceDataViewer::printInvoiceToSerial, &p_svc, &PrintService::printInvoiceToSerial); 
  connect(pdv, &PaymentsDataViewer::paymentVerified, &p_svc, &PrintService::onPaymentCreated); 
  connect(&p_svc, &PrintService::unableToPrint, [this](const QString& m) {
    QMessageBox::warning(this, "Tidak dapat mencetak", m);
  });

  // pengamanan
  auto app = qApp;
  if (QDate::currentDate() >= QDate::fromString("2026-06-20", "yyyy-MM-dd")) {
    QTimer::singleShot(60000, [app]() {
      QMessageBox::information(nullptr, "Aplikasi Kadaluarsa", "Ini adalah versi Pengembang\ndan telah dijadwalkan kadaluarsa tanggal pada 20 Juni 2026");
      qApp->quit();
    });
  }

  // Login
  openLoginForm();
}

MainWindow::~MainWindow() {delete ui;}

#include <QVBoxLayout>

void MainWindow::on_actionLaporanPengeluaranHariIni_triggered()
{
  auto dl = new QDialog(this);
  auto l = new QVBoxLayout(dl);
  auto rv = new ReportView(dl);
  ReportLoader rl(BaseManager::connection);
  auto de = rl.loadDailyExpense(QDate::currentDate());
  rv->showExpenseReport(de);
  dl->setLayout(l);
  l->addWidget(rv);
  dl->setAttribute(Qt::WA_DeleteOnClose);
  rv->resetTransform();
  dl->open();
}

void MainWindow::on_actionLaporanPenjualanHariIni_triggered() {
  auto dl = new QDialog(this);
  auto l = new QVBoxLayout(dl);
  auto rv = new ReportView(dl);
  ReportLoader rl(BaseManager::connection);
  auto de = rl.loadDailySales(QDate::currentDate());
  rv->showSalesReport(de);
  dl->setLayout(l);
  l->addWidget(rv);
  dl->setAttribute(Qt::WA_DeleteOnClose);
  rv->resetTransform();
  dl->open();
}


void MainWindow::setupToolbarActions()
{
    // currently no dynamic action setup is needed, but this function can be used in the future if we want to enable/disable actions based on user role or other conditions
}

void MainWindow::openLoginForm() {
  auto cu = SessionManager::instance().currentUser();
  if (cu.has_value()) {
    return ;
  }
  // hide();
  auto ld = new LoginDialog();
  connect(ld, &LoginDialog::accepted, this, &QWidget::show);
  connect(ld, &LoginDialog::rejected, [this](){
    if (QMessageBox::question(this, "Batal Masuk", "Anda yakin membatalkan masuk ?\nIni akan menutup aplikasi", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) 
      qApp->quit();
    else
      this->openLoginForm();
  });
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
  auto has_super_user = a_man.userHasRole(rec_user.value("id").toInt(), "super_admin");
  ui->actionAdminAdd->setEnabled(has_super_user);
}