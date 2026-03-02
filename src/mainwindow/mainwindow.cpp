#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "src/dialogs/konsumendialog.h"
#include "src/dialogs/createuserdialog.h"
#include "src/dialogs/orderdialog.h"

MainWindow::MainWindow(QWidget *p) :
ui(new Ui::MainWindow), QMainWindow(p) {
  ui->setupUi(this);
  ui->menuToolbar->addAction(ui->addDataToolbar->toggleViewAction());
  ui->menuToolbar->addAction(ui->transactionToolbar->toggleViewAction());
  setupToolbarActions();
}

MainWindow::~MainWindow() {delete ui;}

void MainWindow::setupToolbarActions() {
  connect(ui->actionKonsumenAdd, &QAction::triggered, [this](){
    auto dd = new KonsumenDialog(this);
    dd->setWindowTitle("Tambah data konsumen baru");
    dd->setAttribute(Qt::WA_DeleteOnClose);
    dd->open();
    });

  connect(ui->actionAdminAdd, &QAction::triggered, [this](){
    auto dd = new CreateUserDialog(this);
    dd->setWindowTitle("Daftarkan admin baru");
    dd->setAttribute(Qt::WA_DeleteOnClose);
    dd->open();
    });

  connect(ui->actionOrderCreate, &QAction::triggered, [this](){
    auto dd = new OrderDialog(this);
    dd->prepareCreate();
    dd->setWindowTitle("Form order baru");
    dd->setAttribute(Qt::WA_DeleteOnClose);
    dd->open();
    });
}