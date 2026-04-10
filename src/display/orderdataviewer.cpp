#include "orderdataviewer.h"
#include "ui_dataviewer.h"
#include "src/dialogs/orderdialog.h"

#include <QStyledItemDelegate>
#include <QMenu>
#include <QAction>

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
            option->text = date.toString("dd MMMM yyyy");
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
           date(order_date, 'localtime')
      FROM orders o
     WHERE o.invoice_id IS NULL
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
  ui->dataView->verticalHeader()->hide();
  adjustColumns();
  
  ui->dataView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->dataView, &QTableView::customContextMenuRequested, this, &OrderDataViewer::on_dataView_customContextMenuRequested);
  
  connect(this, &DataViewer::refreshed, ui->dataView, &QTableView::resizeColumnsToContents);
  
  // inisiasi actions
  m_createOrderAction = new QAction(this);
  m_createOrderAction->setObjectName("createOrderAction");
  m_createOrderAction->setToolTip("Buat order baru");
  m_createOrderAction->setText("Order Baru");
  connect(m_createOrderAction, &QAction::triggered, this, &OrderDataViewer::openCreateOrderDialog);
  
}

OrderDataViewer::~OrderDataViewer(){}

void OrderDataViewer::on_dataView_customContextMenuRequested(const QPoint& p) {
  QMenu ctx;
  ctx.setToolTipsVisible(true);

  auto submenu = ctx.addMenu("Data baru");
  submenu->setToolTipsVisible(true);
  submenu->addAction(m_createOrderAction);
  ctx.exec(Ui()->dataView->viewport()->mapToGlobal(p));
}

void OrderDataViewer::openCreateOrderDialog() {
  OrderDialog od;
  connect(&od, &QDialog::accepted, this, &DataViewer::refresh);
  od.exec();
}