#include "invoiceviews.h"
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSortFilterProxyModel>

InvoiceViews::InvoiceViews(QWidget *p)
  : ui(new Ui::InvoiceViews), 
    queryModel(new QSqlQueryModel),
    QWidget(p)
{
  ui->setupUi(this);
  auto db = QSqlDatabase::database("JUST-INV_DB", true);
  queryModel->setQuery(R"--(
    SELECT id,
       customer AS [Nama Konsumen],
       customer_phone AS CP,
       invoice_date AS [Tanggal Invoice],
       total - paid AS Sisa
  FROM invoices
 ORDER BY Sisa DESC,
          invoice_date ASC;
  )--");
  ui->view->setModel(queryModel);
  ui->view->horizontalHeader()->hideColumn(0);
  ui->view->horizontalHeader()->setStretchLastSection(true);
  ui->view->resizeColumnsToContents();
}

InvoiceViews::~InvoiceViews() { delete ui; }

void InvoiceViews::on_view_customContextMenuRequested(const QPoint &p) {}
