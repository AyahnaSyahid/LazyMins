#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "src/dialogs/konsumendialog.h"
#include "src/dialogs/userdialog.h"
#include "src/dialogs/orderdialog.h"
#include "src/dialogs/productdialog.h"
#include "src/utils/sessionmanager.h"

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
  connectCreateActionToFormDialog(ui->actionKonsumenAdd, "Tambah data konsumen baru", [this](){ return new KonsumenDialog(this); }, this);
  connectCreateActionToFormDialog(ui->actionProdukAdd, "Tambah data produk baru", [this](){ return new ProductDialog(this); }, this);
  connectCreateActionToFormDialog(ui->actionOrderCreate, "Buat order baru", [this](){ return new OrderDialog(this); }, this);
  connectCreateActionToFormDialog(ui->actionAdminAdd, "Tambah data admin baru", [this](){ return new UserDialog(this); }, this);
}

MainWindow::~MainWindow() {delete ui;}

void MainWindow::setupToolbarActions() {
  // currently no dynamic action setup is needed, but this function can be used in the future if we want to enable/disable actions based on user role or other conditions
}