#include "invoiceviews.h"
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>

namespace {
  class Delegate : public QStyledItemDelegate
  {
    public:
      Delegate(QObject *parent=nullptr) : QStyledItemDelegate(parent) {}
    protected:
      void initStyleOption(QStyleOptionViewItem *option, const QModelIndex& mi) const override {
        QStyledItemDelegate::(option, mi);
        switch (mi->column()) {
          case 0:
          case 4:
            option.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            break;
          case 2:
          case 3:
            option.displayAlignment = Qt::AlignHCenter | Qt::AlignVCenter;
            break
          default:
            option.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        }
      }
  };
}

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
  ui->view->setItemDelegate(new Delegate(this))
}

InvoiceViews::~InvoiceViews() { delete ui; }

void InvoiceViews::on_view_customContextMenuRequested(const QPoint &p) {}
