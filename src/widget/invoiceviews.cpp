#include "invoiceviews.h"
#include "ui_invoiceviews.h"
#include "../invoiceprinter.h"
#include "../databaseinterface.h"
#include <QSqlQueryModel>
#include <QSqlError>
#include <QMenu>
#include <QAction>
#include <QSqlDatabase>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>

namespace {
  class Delegate : public QStyledItemDelegate {
    public:
      Delegate(QObject *parent=nullptr) : QStyledItemDelegate(parent) {}
      QString displayText(const QVariant& val, const QLocale& loc) const override {
        if (val.metaType() == QMetaType::fromType<qlonglong>()) {
          return QLocale().toString(val.toLongLong());
        }
        return QStyledItemDelegate::displayText(val, loc);
      }
    protected:
      void initStyleOption(QStyleOptionViewItem *option, const QModelIndex& mi) const override {
        QStyledItemDelegate::initStyleOption(option, mi);
        switch (mi.column()) {
          case 0:
          case 4:
            option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            break;
          case 2:
          case 3:
            option->displayAlignment = Qt::AlignHCenter | Qt::AlignVCenter;
            break;
          default:
            option->displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        }
        option->locale = QLocale();
      }
      
  };
}

InvoiceViews::InvoiceViews(QWidget *p)
  : ui(new Ui::InvoiceViews), 
    queryModel(new QSqlQueryModel(this)),
    proxy(new QSortFilterProxyModel(this)),
    RealTimeDataWidget(p)
{
  ui->setupUi(this);
  auto db = QSqlDatabase::database("JUST-INV_DB", true);
  queryModel->setQuery(modelQuery(false), db);
  proxy->setSourceModel(queryModel);
  proxy->setFilterKeyColumn(-1);
  proxy->setFilterCaseSensitivity(Qt::CaseInsensitive );
  connect(ui->filterEdit, &QLineEdit::textChanged, proxy, &QSortFilterProxyModel::setFilterFixedString);
  ui->view->setModel(proxy);
  ui->view->horizontalHeader()->setStretchLastSection(true);
  auto dlg = new Delegate(this);
  ui->view->setItemDelegate(dlg);
  ui->view->resizeColumnsToContents();
  ui->view->sortByColumn(4, Qt::DescendingOrder);
  connect(&DatabaseInterface::instance(), &DatabaseInterface::tableUpdate, this, &InvoiceViews::reloadModelData);
}

InvoiceViews::~InvoiceViews() { delete ui; }

void InvoiceViews::on_view_customContextMenuRequested(const QPoint &p) {
  auto six = proxy->mapToSource(ui->view->indexAt(p));
  if ( !six.isValid() ) return ;
  QMenu menu;
  auto invoiceMenu = menu.addMenu("Invoice");
  invoiceMenu->addSeparator();
  auto reload = invoiceMenu->addAction("&Refresh");
  auto showInvoice = invoiceMenu->addAction("Struk");
  auto e = invoiceMenu->addAction("Edit");
  
  if(six.siblingAtColumn(4).data(Qt::EditRole).toInt() > 0) {
    invoiceMenu->addSeparator();
    auto a = invoiceMenu->addAction("Bayar");
    connect(a, &QAction::triggered, [this, &six]() { emit repaymentRequest(six.siblingAtColumn(0).data(Qt::EditRole).toInt()); });
  }
  auto del = invoiceMenu->addAction("Hapus");
  connect(e, &QAction::triggered, [this, &six]() { emit editInvoiceRequest(six.siblingAtColumn(0).data(Qt::EditRole).toInt()); } );
  connect(reload, &QAction::triggered, [this]() { reloadModelData( {"invoices"} ); });
  connect(showInvoice, &QAction::triggered, this, &InvoiceViews::showPreview);
  menu.exec(ui->view->viewport()->mapToGlobal(p));
}

QString InvoiceViews::modelQuery(bool viewLunas) const {
  if (viewLunas) return R"--(
    SELECT id AS [ID],
       customer AS [Nama Konsumen],
       customer_phone AS CP,
       invoice_date AS [Tanggal Invoice],
       total - paid AS Sisa
  FROM invoices
 WHERE status = 'active'
 ORDER BY Sisa DESC,
          invoice_date ASC;
  )--";
  return R"--(
    SELECT id AS [ID],
       customer AS [Nama Konsumen],
       customer_phone AS CP,
       invoice_date AS [Tanggal Invoice],
       total - paid AS Sisa
  FROM invoices
 WHERE COALESCE(total - paid, 0) != 0 AND status = 'active'
 ORDER BY Sisa DESC,
          invoice_date ASC;
  )--";
}

void InvoiceViews::on_checkBox_toggled(bool l) {
  auto db = QSqlDatabase::database("JUST-INV_DB", true);
  queryModel->setQuery(modelQuery(l), db);
  proxy->invalidate();
  // qDebug() << queryModel->lastError().text();
  ui->view->resizeColumnsToContents();
}

void InvoiceViews::showPreview() {
  auto &ip = InvoicePrinter::instance();
  auto ci = ui->view->currentIndex();
  if(ci.isValid()) {
    auto si = proxy->mapToSource(ci);
    auto rd = ip.receiptPreview(si.siblingAtColumn(0).data(Qt::EditRole).toInt(), this);
    rd->exec();
    rd->deleteLater();
  }
}

void InvoiceViews::reloadModelData(const QList<QString> &tables) {
  if ( tables.indexOf("invoices") != -1 || 
       tables.indexOf("payments") != -1 ||
       tables.indexOf("konsumen") != -1 ||
       tables.count() < 1) {
         on_checkBox_toggled(ui->checkBox->isChecked());
       }
}


