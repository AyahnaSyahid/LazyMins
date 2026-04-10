#include "invoicedataviewer.h"
#include "src/display/ui_dataviewer.h"
#include "src/dialogs/invoicecomposerdialog.h"
#include "src/dialogs/customerpickerdialog.h"

#include <QMenu>
#include <QAction>
#include <QStyledItemDelegate>

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
  
  
  ctx.addMenu(dataBaruMenu);
  connect(ctx.addAction("Refresh"), &QAction::triggered, this, &DataViewer::refresh);
  ctx.exec(ui->dataView->viewport()->mapToGlobal(p));
}