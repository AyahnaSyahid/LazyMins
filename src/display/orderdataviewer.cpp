#include "orderdataviewer.h"
#include "ui_dataviewer.h"
#include <QStyledItemDelegate>
#include <QTimeZone>

namespace {
  class Delegate : public QStyledItemDelegate
  {
    public:
      using QStyledItemDelegate::QStyledItemDelegate;

    protected:
      void initStyleOption(QStyleOptionViewItem *option, const QModelIndex& ix) const override {
        QStyledItemDelegate::initStyleOption(option, ix);
        switch (ix.column()) {
          case 0:{
            option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            break;
          }
          case 3: {
            option->displayAlignment = Qt::AlignCenter;
            break;
          }
          case 4:
          case 5:
          case 6: {
            option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            option->text = QLocale().toString(ix.data().toInt());
            break;
          }
          case 7: {
            option->displayAlignment = Qt::AlignCenter;
            QDateTime date = ix.data().toDateTime();
            date.setTimeZone(QTimeZone(QTimeZone::LocalTime));
            option->text = date.toString("dd/MM/yyyy");
            break;
          }
          default:
            break;
        }
      }
  };
}

OrderDataViewer::OrderDataViewer(QWidget *p) : DataViewer(p)
{
  auto ui = DataViewer::Ui();
  auto mod = &DataViewer::model();
  
  setQueryArgs(R"--(
    SELECT o.id AS id,
           o.customer_name AS customer_name,
           o.customer_phone AS customer_phone,
           order_number,
           o.subtotal AS subtotal,
           o.discount_amount AS discount,
           o.total_amount AS total_amount,
           order_date
      FROM orders o
           LEFT JOIN
           invoices inv ON o.invoice_id = inv.id
     WHERE inv.payment_status <> 'paid' OR 
           inv.id IS NULL
 )--");
  
  setFilterColumnNames( {"order_number", "customer_name"} );
  ui->dataView->setItemDelegate(new Delegate(this));
  mod->setHeaderData(0, Qt::Horizontal, "ID");
  mod->setHeaderData(1, Qt::Horizontal, "Konsumen");
  mod->setHeaderData(2, Qt::Horizontal, "Kontak");
  mod->setHeaderData(3, Qt::Horizontal, "Nomor Order");
  mod->setHeaderData(4, Qt::Horizontal, "Subtotal");
  mod->setHeaderData(5, Qt::Horizontal, "Diskon");
  mod->setHeaderData(6, Qt::Horizontal, "Total");
  mod->setHeaderData(7, Qt::Horizontal, "Tanggal");
  ui->dataView->setEditTriggers(QTableView::NoEditTriggers);
  ui->dataView->resizeColumnsToContents();
  ui->dataView->verticalHeader()->hide();
}

OrderDataViewer::~OrderDataViewer(){}

void OrderDataViewer::on_dataView_customContextMenuRequested(const QPoint& p) {
  
}