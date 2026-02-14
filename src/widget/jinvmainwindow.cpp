#include "jinvmainwindow.h"
#include "ui_jinvmainwindow.h"
#include "invoiceviews.h"
#include "customerview.h"
#include "createinvoicedialog.h"
#include "../databaseinterface.h"

#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QMenuBar>

JINVMainWindow::JINVMainWindow(QWidget *p) :
  ui(new Ui::JINVMainWindow), QMainWindow(p)
{
  ui->setupUi(this);
  auto mbar = menuBar();
  auto createMenu = mbar->addMenu("Buat");
  auto createInvoice = createMenu->addAction("Invoice");
  
  connect(createInvoice, &QAction::triggered, this, &JINVMainWindow::openInvoiceMaker);
  
  auto dockMenu = mbar->addMenu("View");
  dockMenu->setObjectName("dockMenu");
  
  auto customerView = new CustomerView();
  auto invoiceView = new InvoiceViews();
  
  auto &di = DatabaseInterface::instance();
  auto db = QSqlDatabase::database("JUST-INV_DB", true);
  auto si = di.getStoreInfo(db);
  setWindowTitle(QString("%1 - Just Invoice").arg(si.storeName));
  
  installDockable(customerView, Qt::LeftDockWidgetArea, "Konsumen");
  installDockable(invoiceView, Qt::RightDockWidgetArea, "Invoice");
  
};

JINVMainWindow::~JINVMainWindow() { delete ui; }

void JINVMainWindow::installDockable(QWidget *w, const Qt::DockWidgetArea a, const QString& s) {
  auto dock = new QDockWidget(s, this);
  dock->setWidget(w);
  addDockWidget(a, dock);
  findChild<QMenu*>("dockMenu")->addAction(dock->toggleViewAction());
}

void JINVMainWindow::openInvoiceMaker() {
  auto cr = new CreateInvoiceDialog(this);
  cr->setAttribute(Qt::WA_DeleteOnClose);
  cr->open();
}