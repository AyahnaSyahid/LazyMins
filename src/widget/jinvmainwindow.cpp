#include "jinvmainwindow.h"
#include "ui_jinvmainwindow.h"
#include "invoiceviews.h"
#include "customerview.h"
#include "cashflowview.h"
#include "createinvoicedialog.h"
#include "editinvoicedialog.h"
#include "../databaseinterface.h"
#include "realtimedatawidget.h"
#include "storeinfoeditordialog.h"
#include "repaymentinputdialog.h"
#include "paymentdataeditordialog.h"
#include "pencatatpengeluaran.h"
#include "summarywidget.h"

#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QMessageBox>

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
  auto invoiceView  = new InvoiceViews();
  auto cashFlow     = new CashFlowDailyView();
  auto summary      = new SummaryWidget();  
  
  connect(invoiceView, &InvoiceViews::editInvoiceRequest, this, &JINVMainWindow::openInvoiceEditor);
  connect(invoiceView, &InvoiceViews::editPaymentRequest, this, &JINVMainWindow::openPaymentEditor);
  connect(invoiceView, &InvoiceViews::repaymentRequest, this, &JINVMainWindow::openRepaymentDialog);
  
  auto pengaturan = mbar->addMenu("Pengaturan");
  auto storeInfo = pengaturan->addAction("Toko");
  connect(storeInfo, &QAction::triggered, this, &JINVMainWindow::openStoreInfoEditor);
  
  auto &di = DatabaseInterface::instance();
  auto db = QSqlDatabase::database("JUST-INV_DB", true);
  auto si = di.getStoreInfo(db);
  
  auto createCashLog = createMenu->addAction("Catatan Kas");
  connect(createCashLog, &QAction::triggered, [this]() {
    auto p = new PencatatPengeluaran(this);
    p->setAttribute(Qt::WA_DeleteOnClose);
    p->open();
  });
  
  setWindowTitle(QString("%1 - Just Invoice").arg((*si).storeName));
  
  installDockable(summary, Qt::LeftDockWidgetArea, "Ringkasan");
  installDockable(customerView, Qt::LeftDockWidgetArea, "Konsumen");
  installDockable(cashFlow, Qt::LeftDockWidgetArea, "Kas");
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
  auto prs = DatabaseInterface::instance().getInvoiceData(invoice_id);
  if (!prs) {
    QMessageBox::information(this, "Kesalahan", "Data invoice tidak ditemukan");
    return;
  }
  auto eid = new EditInvoiceDialog(invoice_id, this);
  eid->setAttribute(Qt::WA_DeleteOnClose);
  eid->open();
}

void JINVMainWindow::openPaymentEditor(int invoice_id) {
  auto prs = DatabaseInterface::instance().getPaymentRecords(invoice_id);
  if (prs.isEmpty()) {
    QMessageBox::information(this, "Kesalahan", "Tidak ditemukan pembayaran untuk invoice ini");
    return;
  }
  auto pde = new PaymentDataEditorDialog(invoice_id, this);
  pde->setAttribute(Qt::WA_DeleteOnClose);
  pde->open();
}

void JINVMainWindow::openRepaymentDialog(int invoice_id) {
  auto prs = DatabaseInterface::instance().getInvoiceData(invoice_id);
  if (!prs) {
    QMessageBox::information(this, "Kesalahan", "Data invoice tidak ditemukan");
    return;
  }
  auto ind = *prs;
  if (ind.total == ind.paid) {
    QMessageBox::information(this, "Kesalahan", "Maaf, Invoice ini telah lunas");
    return ;
  }
  auto rep = new RepaymentInputDialog(invoice_id, this);
  rep->setAttribute(Qt::WA_DeleteOnClose);
  rep->open();
}