#include "invoicebrowser.h"

#include <QAction>
#include <QMenu>

#include "ui_dataviewer.h"
#include "src/customs/invoicebrowserdelegate.h"

InvoiceBrowser::InvoiceBrowser(QWidget* parent) : DataViewer(parent) {
  setObjectName("invoiceBrowser");
  QString query = R"-(
SELECT inv.id,
       inv.invoice_number,
       inv.customer_name,
       inv.customer_phone,
       pl.level_name,
       inv.subtotal,
       inv.discount_amount,
       inv.tax_amount,
       inv.total_amount,
       inv.paid_amount,
       inv.remaining_amount,
       inv.staging_status,
       inv.settlement_status,
       inv.revision_no,
       inv.parent_id,
       inv.is_active,
       date(inv.issue_date, 'localtime'),
       date(inv.due_date, 'localtime'),
       date(inv.created_at, 'localtime'),
       date(inv.updated_at, 'localtime'),
       inv.notes,
       inv.internal_notes
  FROM invoices inv
       JOIN
       price_levels pl ON inv.price_level_id = pl.id
    )-";
  setQueryArgs(query);
  model().setHeaderData(0, Qt::Horizontal, "ID", Qt::DisplayRole);
  model().setHeaderData(1, Qt::Horizontal, "Nomor", Qt::DisplayRole);
  model().setHeaderData(2, Qt::Horizontal, "Konsumen", Qt::DisplayRole);
  model().setHeaderData(3, Qt::Horizontal, "Kontak", Qt::DisplayRole);
  model().setHeaderData(4, Qt::Horizontal, "Tipe Harga", Qt::DisplayRole);
  model().setHeaderData(5, Qt::Horizontal, "Subtotal", Qt::DisplayRole);
  model().setHeaderData(6, Qt::Horizontal, "Diskon", Qt::DisplayRole);
  model().setHeaderData(7, Qt::Horizontal, "Pajak", Qt::DisplayRole);
  model().setHeaderData(8, Qt::Horizontal, "Total", Qt::DisplayRole);
  model().setHeaderData(9, Qt::Horizontal, "Terbayar", Qt::DisplayRole);
  model().setHeaderData(10, Qt::Horizontal, "Sisa", Qt::DisplayRole);
  model().setHeaderData(11, Qt::Horizontal, "S INV", Qt::DisplayRole);
  model().setHeaderData(12, Qt::Horizontal, "S Bayar", Qt::DisplayRole);
  model().setHeaderData(13, Qt::Horizontal, "No Rev", Qt::DisplayRole);
  model().setHeaderData(14, Qt::Horizontal, "INV ASAL", Qt::DisplayRole);
  model().setHeaderData(15, Qt::Horizontal, "Aktif", Qt::DisplayRole);
  model().setHeaderData(16, Qt::Horizontal, "Terbit", Qt::DisplayRole);
  model().setHeaderData(17, Qt::Horizontal, "Tagih", Qt::DisplayRole);
  model().setHeaderData(18, Qt::Horizontal, "Pembuatan", Qt::DisplayRole);
  model().setHeaderData(19, Qt::Horizontal, "Pembaruan", Qt::DisplayRole);
  model().setHeaderData(20, Qt::Horizontal, "Notes", Qt::DisplayRole);
  model().setHeaderData(21, Qt::Horizontal, "I Notes", Qt::DisplayRole);

  setColumnVisible(0, false);
  setFilterColumnNames({"invoice_number", "customer_name"});
  Ui()->dataView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(Ui()->dataView, &QAbstractItemView::customContextMenuRequested, this,
          &InvoiceBrowser::on_dataView_customContextMenuRequested);
  Ui()->dataView->setItemDelegate(new InvoiceBrowserDelegate(this));
  Ui()->dataView->resizeColumnsToContents();
}

void InvoiceBrowser::on_dataView_customContextMenuRequested(const QPoint& p) {
  QMenu ctx;
  ctx.setToolTipsVisible(true);

  auto printMenu = ctx.addMenu("Print");
  auto serial = printMenu->addAction("Print to Thermal");
  serial->setEnabled(false);

  auto ix = Ui()->dataView->indexAt(p);
  if (ix.isValid()) {
    serial->setEnabled(true);
    int id = ix.siblingAtColumn(0).data().toInt();
    connect(serial, &QAction::triggered,
            [this, id]() { emit this->serialPrintRequested(id); });
  }
  ctx.exec(Ui()->dataView->viewport()->mapToGlobal(p));
}