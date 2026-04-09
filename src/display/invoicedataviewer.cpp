#include "invoicedataviewer.h"
#include "src/display/ui_dataviewer.h"
#include "src/dialogs/invoicecomposerdialog.h"
#include "src/dialogs/customerpickerdialog.h"

#include <QMenu>
#include <QAction>

InvoiceDataViewer::InvoiceDataViewer(QWidget *p):
DataViewer(p)
{
  ui = DataViewer::Ui();
  setQueryArgs(R"-(
    SELECT i.id, i.invoice_number, i.customer_name, i.remaining_amount, i.issue_date, i.due_date
      FROM invoices i WHERE is_active = 1 AND settlement_status <> 'paid' AND staging_status <> 'canceled'
  )-");
  auto m = &model();

  m->setHeaderData(0, Qt::Horizontal, "ID");
  m->setHeaderData(1, Qt::Horizontal, "Nomor");
  m->setHeaderData(2, Qt::Horizontal, "Konsumen");
  m->setHeaderData(3, Qt::Horizontal, "Sisa");
  m->setHeaderData(4, Qt::Horizontal, "Pembuatan");
  m->setHeaderData(5, Qt::Horizontal, "Penagihan");

  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, &InvoiceDataViewer::customContextMenuRequested, this, &InvoiceDataViewer::openContextMenu);

  ui->dataView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  
  m_createInvoiceAction = new QAction(this);
  m_createInvoiceAction->setText("Buat Invoice");
  connect(m_createInvoiceAction, &QAction::triggered, this, &InvoiceDataViewer::onCreateInvoice);
}

InvoiceDataViewer::~InvoiceDataViewer() {}

void InvoiceDataViewer::onCreateInvoice() {
  InvoiceComposerDialog ids(this);
  connect(&ids, &QDialog::accepted, this, &DataViewer::refresh);
  ids.exec();
}

void InvoiceDataViewer::openContextMenu(const QPoint& p) {
  QMenu ctx;
  ctx.setToolTipsVisible(true);
  auto subm = ctx.addMenu("Data baru");
  subm->setToolTipsVisible(true);
  subm->addAction(m_createInvoiceAction);
  ctx.exec(mapToGlobal(p));
}

