#include "mainwindow.h"

#include <QDate>
#include <QDockWidget>
#include <QMessageBox>
#include <QTimer>

#include "actiongroup.h"
#include "mainwindowcontext.h"
#include "src/dialogs/configureserialposdialog.h"
#include "src/dialogs/edituserdialog.h"
#include "src/dialogs/instantorderdialog.h"
#include "src/dialogs/kategoriprodukdialog.h"
#include "src/dialogs/konsumendialog.h"
#include "src/dialogs/logindialog.h"
#include "src/dialogs/orderdialog.h"
#include "src/dialogs/productdialog.h"
#include "src/dialogs/revokepassworddialog.h"
#include "src/dialogs/userdialog.h"
#include "src/display/adminsviewer.h"
#include "src/display/akuntransaksidataviewer.h"
#include "src/display/finishingservicesviewer.h"
#include "src/display/invoicedataviewer.h"
#include "src/display/konsumendataviewer.h"
#include "src/display/orderdataviewer.h"
#include "src/display/paymentsdataviewer.h"
#include "src/display/produkdataviewer.h"
#include "src/managers/appsettingsmanager.h"
#include "src/modul_laporan/reportdialog.h"
#include "src/modul_laporan/reportloader.h"
#include "src/modul_laporan/reportview.h"
#include "src/printer/printservice.h"
#include "src/utils/posprintertestdialog.h"
#include "src/utils/sessionmanager.h"
#include "ui_mainwindow.h"

namespace {
void connectCreateActionToFormDialog(QAction* action, const QString& formTitle,
                                     std::function<FormDialog*()> formOpener,
                                     QObject* me = nullptr) {
  me->connect(action, &QAction::triggered, [me, formTitle, formOpener]() {
    auto dd = formOpener();
    dd->prepareCreate();
    dd->setWindowTitle(formTitle);
    dd->setAttribute(Qt::WA_DeleteOnClose);
    dd->open();
  });
}
void connectModifyActionToFormDialog(QAction* action, const QString& formTitle,
                                     std::function<FormDialog*()> formOpener,
                                     std::function<QSqlRecord()> recordProvider,
                                     QObject* me = nullptr) {
  me->connect(action, &QAction::triggered,
              [me, formTitle, formOpener, recordProvider]() {
                auto dd = formOpener();
                dd->prepareModify(recordProvider());
                dd->setWindowTitle(formTitle);
                dd->setAttribute(Qt::WA_DeleteOnClose);
                dd->open();
              });
}
}  // namespace

MainWindow::MainWindow(QWidget* p) : QMainWindow(p), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  ui->menuToolbar->addAction(ui->addDataToolbar->toggleViewAction());
  ui->menuToolbar->addAction(ui->transactionToolbar->toggleViewAction());
  auto dockSetup = [](QDockWidget* dw, const QString& title,
                      QWidget* widget) -> QDockWidget* {
    dw->setWidget(widget);
    dw->setWindowTitle(title);
    return dw;
  };

  context = new MainWindowContext(this, menuBar(), this);

  auto dv1 = new ProdukDataViewer;
  dv1->initialize(context);

  auto fs1 = new FinishingServicesViewer;
  fs1->initialize(context);
  
  auto ord1 = new OrderDataViewer;
  ord1->initialize(context);

  connect(ui->actionOrderCreate, &QAction::triggered, ord1,
          &OrderDataViewer::openCreateOrderDialog);

  connect(ord1, &OrderDataViewer::orderCreated, dv1, &DataViewer::refresh);
  connect(ord1, &OrderDataViewer::stockChanged, dv1, &DataViewer::refresh);

  auto idv = new InvoiceDataViewer;
  idv->initialize(context);

  connect(ui->actionInvoiceCreate, &QAction::triggered, idv,
          &InvoiceDataViewer::onCreateInvoice);
  connect(idv, &DataViewer::refreshed, ord1,
          &DataViewer::refresh);  // Hati2 jangan sampai circular
  connect(ord1, &OrderDataViewer::createInvoiceRequested, idv,
          &InvoiceDataViewer::createInvoiceForOrder);

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
  connect(ui->actionKonsumenAdd, &QAction::triggered, kdv,
          &KonsumenDataViewer::openCreateKonsumenDialog);

  auto pdv = new PaymentsDataViewer;
  pdv->initialize(context);
  pdv->setPageSize(100);
  pdv->refresh();

  // InvoiceDataViewer bisa membuat pembayaran
  connect(idv, &InvoiceDataViewer::paymentCreated, pdv, &DataViewer::refresh);
  connect(idv, &InvoiceDataViewer::paymentCreated, atdv, &DataViewer::refresh);

  // PaymentsDataViewer bisa memverifikasi pembayaran
  connect(pdv, &PaymentsDataViewer::paymentVerified, idv, &DataViewer::refresh);
  connect(pdv, &PaymentsDataViewer::paymentVerified, atdv,
          &DataViewer::refresh);

  // tabifyDockWidget(dsPy, dsF); // products, finishings
  // dsPy->raise();

  // tabifyDockWidget(dsO, dsI);  // orders, invoices
  // tabifyDockWidget(dsI, dsPy); // invoices, payments
  // dsO->raise();

  tabifyDockWidget(dsK, dsAT);  // AkunTransaksi, konsumen
  dsK->raise();

  connectCreateActionToFormDialog(
      ui->actionAdminAdd, "Tambah data admin baru",
      [this]() {
        auto ud = new UserDialog(this);
        return ud;
      },
      this);
  connect(ui->actionInstantOrderCreate, &QAction::triggered,
          [this, dv1, idv, atdv]() {
            auto dialog = new InstantOrderDialog(this);
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            connect(dialog, &QDialog::accepted, dv1, &DataViewer::refresh);
            connect(dialog, &QDialog::accepted, idv, &DataViewer::refresh);
            connect(dialog, &QDialog::accepted, atdv, &DataViewer::refresh);
            dialog->open();
          });

  context->craftAll();
  // Various actions
  auto actionGroup = new ActionGroup(this);
  auto _menuTambah = context->getOrCreateMenu("Data/Tambah");
  actionGroup->setRootWidget(this);

  _menuTambah->addSeparator();
  _menuTambah->addAction(actionGroup->buatAkunTransaksiAction);
  _menuTambah->addAction(actionGroup->catatPengeluaranAction);

  connect(actionGroup, &ActionGroup::newAkunTransaksiCreated, atdv,
          &AkunTransaksiDataViewer::refresh);
  connect(actionGroup, &ActionGroup::expenseAdded, atdv,
          &AkunTransaksiDataViewer::refresh);
  // UserSession
  auto& sm = SessionManager::instance();
  connect(&sm, &SessionManager::loginSuccess, this,
          &MainWindow::currentUserChanged);
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
      return;
    }
    auto ud = new EditUserDialog("", this);
    if (!ud->setUser(sm.currentUser()->value("username").toString()) ||
        !ud->userLoaded()) {
      QMessageBox::warning(this, "Peringatan",
                           "Username tidak ditemukan dalam database");
      ud->deleteLater();
      return;
    }
    if (!sm.isSuperAdminSession()) ud->setEditRoleDisabled();
    ud->setAttribute(Qt::WA_DeleteOnClose);
    ud->open();
  });
  auto actBrowseAkun = new QAction("Lihat", this);
  actBrowseAkun->setObjectName("browseAkunAction");
  ui->menuAkun->insertAction(actEditAkun, actBrowseAkun);
  connect(actBrowseAkun, &QAction::triggered, this,
          &MainWindow::onBrowseAccounts);

  // Window Title
  AppSettingsManager apm;
  setWindowTitle(
      apm.getSettings("company_name").value("setting_value").toString() +
      "- LazyAdmins");

  // Printer Stuff
  ui->actionPrinterTest_2->setEnabled(false);
  connect(ui->actionPrinterTest_2, &QAction::triggered, [this]() {
    PosPrinterTestDialog* pp = new PosPrinterTestDialog(this);
    pp->open();
  });
  connect(ui->actionEscPosConfig, &QAction::triggered, [this]() {
    auto* pp = new ConfigureSerialPosDialog(this);
    pp->exec();
  });

  auto& p_svc = PrintService::instance();
  connect(idv, &InvoiceDataViewer::paymentCreated, &p_svc,
          &PrintService::onPaymentCreated);
  connect(idv, &InvoiceDataViewer::printInvoiceToSerial, &p_svc,
          &PrintService::printInvoiceToSerial);
  connect(pdv, &PaymentsDataViewer::paymentVerified, &p_svc,
          &PrintService::onPaymentCreated);
  connect(&p_svc, &PrintService::unableToPrint, [this](const QString& m) {
    QMessageBox::warning(this, "Tidak dapat mencetak", m);
  });
  setupAutoPrintStuctAction();
  // pengamanan
  auto app = qApp;
  if (QDate::currentDate() >= QDate::fromString("2027-01-01", "yyyy-MM-dd")) {
    QTimer::singleShot(60000, [app]() {
      QMessageBox::information(
          nullptr, "Aplikasi Kadaluarsa",
          "Ini adalah versi Pengembang\ndan telah dijadwalkan kadaluarsa "
          "tanggal pada 20 Juni 2026");
      qApp->quit();
    });
  }

  // Session
  connect(&sm, &SessionManager::idleTimeout, [this]() {
    auto& csm = SessionManager::instance();
    if (csm.currentUserId() > 0) {
      RevokePasswordDialog rv(this);
      if (QDialog::Accepted == rv.exec()) {
        csm.restartIdleTimer();
      } else {
        csm.logout();
      }
    }
  });

  // Login
  openLoginForm();
}

MainWindow::~MainWindow() { delete ui; }

#include <QVBoxLayout>

void MainWindow::on_actionLaporanPengeluaranHariIni_triggered() {
  auto dl = new ReportDialog(BaseManager::connection,
                             SessionManager::instance().currentUserId(), this);
  dl->setMode(ReportDialog::ExpenseMode);
  dl->setAttribute(Qt::WA_DeleteOnClose);
  dl->open();
}

void MainWindow::on_actionLaporanPenjualanHariIni_triggered() {
  auto dl = new ReportDialog(BaseManager::connection,
                             SessionManager::instance().currentUserId(), this);
  dl->setAttribute(Qt::WA_DeleteOnClose);
  dl->open();
}

#include "src/dialogs/infopercetakandialog.h"
void MainWindow::on_actionInfoPercetakan_triggered() {
  InfoPercetakanDialog ipd;
  ipd.exec();
}

void MainWindow::on_actionTentangQt_triggered() {
  QMessageBox::aboutQt(this, "Tentang Qt");
}

void MainWindow::onBrowseAccounts() {
  if (!SessionManager::instance().isSuperAdminSession()) {
    QMessageBox::warning(this, "Tidak dapat melihat/edit akun",
                         "Hanya super admin yang dapat melihat daftar akun");
    return;
  }
  auto wd = new QDialog(this);
  auto l = new QVBoxLayout(wd);
  auto rv = new AdminsViewer(wd);
  l->addWidget(rv);
  wd->setLayout(l);
  wd->setWindowTitle("Daftar Akun");
  wd->setAttribute(Qt::WA_DeleteOnClose);
  wd->open();
}

void MainWindow::setupToolbarActions() {
  // currently no dynamic action setup is needed, but this function can be used
  // in the future if we want to enable/disable actions based on user role or
  // other conditions
}

#include <QSettings>
void MainWindow::setupAutoPrintStuctAction() {
  QSettings settings;
  bool autoPrintStructDisabled =
      settings.value("printer/disableAutoPrint", false).toBool();
  ui->actionAutoPrintStruct->setChecked(!autoPrintStructDisabled);
  connect(ui->actionAutoPrintStruct, &QAction::triggered, [this](bool checked) {
    PrintService::instance().enableAutoPrint(checked);
  });
}

void MainWindow::openLoginForm() {
  auto cu = SessionManager::instance().currentUser();
  if (cu.has_value()) {
    return;
  }
  // hide();
  auto ld = new LoginDialog(this);
  ld->adjustSize();
  connect(ld, &LoginDialog::accepted, this, &QWidget::show);
  ld->setAttribute(Qt::WA_DeleteOnClose);
  ld->open();
}

void MainWindow::currentUserChanged() {
  auto& sm = SessionManager::instance();
  auto opt_user = sm.currentUser();
  if (!opt_user) {
    QMessageBox::critical(
        this, "Fatal Error",
        "Tidak dapat mendeteksi user valid !!\nApplikasi akan di terminasi");
    qApp->quit();
    return;
  }
  auto rec_user = *opt_user;
  AdminManager a_man;
  auto has_super_user =
      a_man.userHasRole(rec_user.value("id").toInt(), "super_admin");
  ui->actionAdminAdd->setEnabled(has_super_user);
}