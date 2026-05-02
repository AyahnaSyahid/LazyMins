#include "invoicedataviewer.h"
#include "src/display/ui_dataviewer.h"
#include "src/dialogs/invoicecomposerdialog.h"
#include "src/dialogs/customerpickerdialog.h"
#include "src/dialogs/paymentdialog.h"
#include "src/utils/sessionmanager.h"
#include "src/display/invoicebrowser.h"

#include "src/managers/managers.h"
#include "src/managers/financialledgerservice.h"

#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QSqlField>
#include <QSqlRecord>

namespace {
  class Delegate : public QStyledItemDelegate
  {
    public:
      using QStyledItemDelegate::QStyledItemDelegate;
      
    protected:
      void initStyleOption(QStyleOptionViewItem *option, const QModelIndex& i) const override {
        QStyledItemDelegate::initStyleOption(option, i);
        switch (i.column()) {
          case 0:
          case 1:
          case 4:
          case 5:
            option->displayAlignment = Qt::AlignCenter;
            break;
          case 3:
            option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            option->text = QString("%L1").arg(i.data().toInt());
        }
      }
  };
}

InvoiceDataViewer::InvoiceDataViewer(QWidget *p):
DataViewer(p)
{
  ui = DataViewer::Ui();
  setQueryArgs(R"-(
    SELECT i.id, i.invoice_number, i.customer_name, i.remaining_amount, date(i.issue_date, 'localtime'), date(i.due_date, 'localtime')
      FROM invoices i WHERE is_active = 1 AND settlement_status <> 'paid' AND staging_status <> 'canceled'
  )-");
  auto m = &model();

  m->setHeaderData(0, Qt::Horizontal, "ID");
  m->setHeaderData(1, Qt::Horizontal, "Nomor");
  m->setHeaderData(2, Qt::Horizontal, "Konsumen");
  m->setHeaderData(3, Qt::Horizontal, "Sisa");
  m->setHeaderData(4, Qt::Horizontal, "Pembuatan");
  m->setHeaderData(5, Qt::Horizontal, "Penagihan");
  
  setFilterColumnNames({ "invoice_number", "customer_name"});
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, &InvoiceDataViewer::customContextMenuRequested, this, &InvoiceDataViewer::openContextMenu);
  connect(ui->dataView, &QTableView::customContextMenuRequested, this, &InvoiceDataViewer::on_dataView_customContextMenuRequested);
  connect(this, &DataViewer::refreshed, ui->dataView, &QTableView::resizeColumnsToContents);

  ui->dataView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->dataView->setItemDelegate(new Delegate(this));
  ui->dataView->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->dataView->verticalHeader()->hide();
  
  m_createInvoiceAction = new QAction(this);
  m_createInvoiceAction->setText("Buat Invoice");
  connect(m_createInvoiceAction, &QAction::triggered, this, &InvoiceDataViewer::onCreateInvoice);
  
  // Menu Data Baru
  dataBaruMenu = new QMenu(this);
  dataBaruMenu->setObjectName("dataBaruMenu");
  dataBaruMenu->setToolTipsVisible(true);
  dataBaruMenu->setTitle("Data Baru");
  dataBaruMenu->addAction(m_createInvoiceAction);
}

InvoiceDataViewer::~InvoiceDataViewer() {}

void InvoiceDataViewer::onCreateInvoice() {
  InvoiceComposerDialog ids(this);
  connect(&ids, &QDialog::accepted, this, &DataViewer::refresh);
  // Signal Forwarding
  connect(&ids, &InvoiceComposerDialog::invoiceCreated, this, &InvoiceDataViewer::invoiceCreated);
  connect(&ids, &InvoiceComposerDialog::paymentCreated, this, &InvoiceDataViewer::paymentCreated);
  ids.exec();
}

void InvoiceDataViewer::openContextMenu(const QPoint& p) {
  QMenu ctx;
  ctx.setToolTipsVisible(true);
  
  ctx.addMenu(dataBaruMenu);
  connect(ctx.addAction("Refresh"), &QAction::triggered, this, &DataViewer::refresh);
  ctx.exec(mapToGlobal(p));
}

void InvoiceDataViewer::on_dataView_customContextMenuRequested(const QPoint& p) {
  QMenu ctx;
  ctx.setToolTipsVisible(true);
  
  auto clickedIndex = ui->dataView->indexAt(p);
  if (clickedIndex.isValid()) {
    int invoiceId = clickedIndex.siblingAtColumn(0).data().toInt();
    auto createPaymentAction = ctx.addAction("Atur Pembayaran");
    connect(createPaymentAction, &QAction::triggered, [this, invoiceId]() { openPaymentForInvoice(invoiceId); });
    
    ctx.addSeparator();
  } 
  ctx.addMenu(dataBaruMenu);
  connect(ctx.addAction("Refresh"), &QAction::triggered, this, &DataViewer::refresh);

  ctx.addSeparator();

  auto browse = ctx.addAction("Browse");
  connect(browse, &QAction::triggered, this, &InvoiceDataViewer::onBrowseInvoices);

  auto printMenu = ctx.addMenu("Print");
  auto serial = printMenu->addAction("Print to Thermal");
  serial->setEnabled(false);

  ctx.exec(ui->dataView->viewport()->mapToGlobal(p));
}

void InvoiceDataViewer::openPaymentForInvoice(int invoiceId)
{
  PaymentDialog pd(this);
  pd.setInvoiceId(invoiceId);
  connect(&pd, &PaymentDialog::paymentGranted, this, &InvoiceDataViewer::onPaymentGranted);
  pd.exec();
}

void InvoiceDataViewer::createInvoiceForOrder(int orderId) {
  OrderManager oman;
  auto optOrder = oman.getById(orderId);
  if(!optOrder) {
    QMessageBox::information(this, "Kesalahan", "Sistem tidak dapat menemukan order");
    return ;
  }
  if (!(optOrder->value("invoice_id").isNull())) {
    QMessageBox::information(this, "Kesalahan", "Order ini sudah memiliki invoice");
    return ;
  }
  KonsumenManager km;
  auto optCustomer = km.getById(optOrder->value("customer_id").toInt());
  QSqlRecord walkIn;
  if (!optCustomer) {
    walkIn = QSqlRecord();
    walkIn.append(QSqlField("id", optOrder->value("customer_id").metaType(), "orders"));
    walkIn.append(QSqlField("nama_lengkap", optOrder->value("customer_name").metaType(), "orders"));
    walkIn.append(QSqlField("nomor_telp", optOrder->value("customer_phone").metaType(), "orders"));
    walkIn.setValue(0, optOrder->value("customer_id"));
    walkIn.setValue(1, optOrder->value("customer_name"));
    walkIn.setValue(2, optOrder->value("customer_phone"));
  } else {
    walkIn = *optCustomer;
  }

  InvoiceComposerDialog ids(this);
  ids.setCustomer(walkIn);
  ids.importOrders({orderId});
  connect(&ids, &QDialog::accepted, this, &DataViewer::refresh);
  
  // Signal Forwarding
  connect(&ids, &InvoiceComposerDialog::invoiceCreated, this, &InvoiceDataViewer::invoiceCreated);
  connect(&ids, &InvoiceComposerDialog::paymentCreated, this, &InvoiceDataViewer::paymentCreated);
  ids.exec();
}

void InvoiceDataViewer::onPaymentGranted(const QVariantMap& vm)
{
  qDebug() << "Payment Processed by InvoiceDataViewer";
  if (!vm.contains("invoice_id")) {
    qDebug() << "Invoice ID tidak ada dalam parameter";
    return ;
  }
  auto user = SessionManager::instance().currentUser();
  if (!user.has_value()) {
    QMessageBox::critical(this, "Akses ditolak", "Error:\nTidak ada aktif user dalam sesi ini\nTapi mengapa anda bisa masuk sampai sini ??");
    return ;
  }
  auto userRec = *user;
  int invoiceId = vm["invoice_id"].toInt();

  QVariantMap withUser(vm);
  withUser["admin_id"] = userRec.value("id");
  
  FinancialLedgerService flc;
  int createdPaymentId = -1;
  if (!flc.createPayment(withUser, &createdPaymentId)) {
    QMessageBox::critical(this, "Pembayaran gagal", "Error:\n" + flc.errorString());
    return ;
  }
  emit paymentCreated(createdPaymentId);
  refresh();
}

void InvoiceDataViewer::onBrowseInvoices() {
  auto ib = new InvoiceBrowser();
  ib->setWindowTitle("Data Invoice");
  ib->setAttribute(Qt::WA_DeleteOnClose);
  connect(ib, &InvoiceBrowser::serialPrintRequested, this, &InvoiceDataViewer::printInvoiceToSerial);
  ib->show();
}