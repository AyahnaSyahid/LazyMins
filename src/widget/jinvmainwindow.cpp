#include "jinvmainwindow.h"
#include "ui_jinvmainwindow.h"
#include "invoiceviews.h"
#include "customerview.h"
#include "createinvoicedialog.h"
#include "editinvoicedialog.h"
#include "../databaseinterface.h"
#include "realtimedatawidget.h"
#include "storeinfoeditordialog.h"
#include "repaymentinputdialog.h"

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
  createInvoice->setShortcut(QKeySequence::New);
  connect(createInvoice, &QAction::triggered, this, &JINVMainWindow::openInvoiceMaker);
  
  auto dockMenu = mbar->addMenu("View");
  dockMenu->setObjectName("dockMenu");
  
  auto customerView = new CustomerView();
  auto invoiceView = new InvoiceViews();
  
  connect(invoiceView, &InvoiceViews::editInvoiceRequest, this, &JINVMainWindow::openInvoiceEditor);
  connect(invoiceView, &InvoiceViews::repaymentRequest, this, &JINVMainWindow::openRepaymentDialog);
  
  auto pengaturan = mbar->addMenu("Pengaturan");
  auto storeInfo = pengaturan->addAction("Toko");
  connect(storeInfo, &QAction::triggered, this, &JINVMainWindow::openStoreInfoEditor);
  
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

void JINVMainWindow::openStoreInfoEditor() {
  auto sie = new StoreInfoEditorDialog(this);
  sie->setAttribute(Qt::WA_DeleteOnClose);
  sie->open();
}

void JINVMainWindow::openInvoiceEditor(int invoice_id) {
  auto eid = new EditInvoiceDialog(invoice_id, this);
  eid->setAttribute(Qt::WA_DeleteOnClose);
  eid->open();
}

void JINVMainWindow::openRepaymentDialog(int invoice_id) {
  auto rep = new RepaymentInputDialog(invoice_id, this);
  rep->setAttribute(Qt::WA_DeleteOnClose);
  rep->open();
}