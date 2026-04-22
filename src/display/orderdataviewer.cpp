#include "orderdataviewer.h"
#include "ui_dataviewer.h"
#include "src/dialogs/orderdialog.h"

#include <QStyledItemDelegate>
#include <QMenu>
#include <QAction>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextOption>

#include <QTimeZone>

namespace {
  const QBrush processingBrush(QColor(255, 255, 200));
  const QBrush pendingBrush(QColor(255, 200, 200));
  const QBrush readyBrush(QColor(200, 255, 200));
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
          case 8: {
            option->displayAlignment = Qt::AlignCenter;
            break;
          }
          default:
            break;
        }
        auto status = ix.siblingAtColumn(8).data(Qt::DisplayRole).toString();
        if (status == "pending")
          option->backgroundBrush = pendingBrush;
        else if (status == "processing")
          option->backgroundBrush = processingBrush;
        else if (status == "ready")
          option->backgroundBrush = readyBrush;
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
           date(order_date, 'localtime'),
           o.staging_status AS status
      FROM orders o
     WHERE o.invoice_id IS NULL OR ( o.staging_status <> 'completed' AND o.staging_status <> 'cancelled' )
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
  mod->setHeaderData(8, Qt::Horizontal, "Status");
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

void OrderDataViewer::setOrderStatus(const QModelIndex &ix, const QString &status)
{
  auto &mod = DataViewer::model();
  int paymentId = ix.siblingAtColumn(0).data().toInt();
  PaymentManager paym;
  
}

void OrderDataViewer::viewOrderItems(const QModelIndex &ix)
{
  auto dl = new QDialog(this);
  dl->setWindowTitle("Detail Pesanan");
  dl->setAttribute(Qt::WA_DeleteOnClose);
  auto layout = new QVBoxLayout(dl);
  auto pte = new QPlainTextEdit(dl);
  layout->addWidget(pte);
  OrderItemManager oim;
  auto order_items = oim.getByOrder(ix.siblingAtColumn(0).data().toInt());
  pte->setReadOnly(true);
  pte->setWordWrapMode(QTextOption::NoWrap);
  if (order_items.isEmpty()) {
    pte->setPlainText("Error:\nData items tidak ditemukan");
    dl->open();
    return ;
  }
  OrderItemFinishingManager oifm;
  QList<QList<QSqlRecord>> item_finishings_records;

  for(int a=0; a<order_items.size(); a++) {
    auto item = order_items.at(a);
    auto fs = oifm.getByOrderItem(item.value("id").toInt());
    item_finishings_records << fs;
  }
  auto formatBold = QTextCharFormat();
  formatBold.setFontWeight(QFont::Bold);
  auto formatThin = QTextCharFormat();
  formatThin.setFontWeight(QFont::Normal);

  for(int a=0; a<order_items.size(); a++) {
    auto item = order_items.at(a);
    QString itemString = QString("[%1] %2\n").arg(item.value("sku").toString(), item.value("product_name").toString());
    QString itemInfo   = QString("%1 %2 x %3\n").arg(item.value("quantity").toInt())
                                              .arg(item.value("unit").toString())
                                              .arg(item.value("sale_price").toInt());
    if (item.value("use_area").toBool()) {
      itemInfo = QString("%L1x%L2 %L3 x%L4 x%L5\n").arg(item.value("size_width").toDouble(), 0, 'f', 2)
                                         .arg(item.value("size_height").toDouble(), 0, 'f', 2)
                                         .arg(item.value("unit").toString())
                                         .arg(item.value("sale_price").toInt())
                                         .arg(item.value("quantity").toInt());
    }
    auto cursor = pte->textCursor();
    pte->setTextCursor(cursor);
    cursor.insertText(itemString, formatBold);
    cursor.insertText(itemInfo, formatThin);
    auto finishings = item_finishings_records.at(a);
    for(auto const& finishing : finishings) {
      QString finishingString = QString(" - [%1] %L2x%L3\n").arg(finishing.value("finishing_name").toString())
                                    .arg(finishing.value("quantity").toInt())
                                    .arg(finishing.value("price").toInt());
      cursor.insertText(finishingString, formatThin);
      cursor.insertBlock();
    }
  }
  dl->adjustSize();
  dl->open();
}

QString OrderDataViewer::orderStatus(const QModelIndex &index) const
{
    auto mod = index.model();
    return mod->data(index.siblingAtColumn(8)).toString();
}

void OrderDataViewer::on_dataView_customContextMenuRequested(const QPoint& p) {
  QMenu ctx;
  ctx.setToolTipsVisible(true);

  auto currentIndex = Ui()->dataView->indexAt(p);
  if (currentIndex.isValid()) {
    auto viewOrderItems = ctx.addAction("Lihat Pesanan");
    auto substatus = ctx.addMenu("Set Status");
    auto setReadyAction = substatus->addAction("Ready");
    auto setCompletedAction = substatus->addAction("Completed");
    ctx.addSeparator();

    connect(viewOrderItems, &QAction::triggered, [this, currentIndex]() { this->viewOrderItems(currentIndex); });
    connect(setReadyAction, &QAction::triggered, [this, currentIndex]() { setOrderStatus(currentIndex, "ready"); });
    connect(setCompletedAction, &QAction::triggered, [this, currentIndex]() { setOrderStatus(currentIndex, "completed"); });
  }

  auto submenu = ctx.addMenu("Data baru");
  submenu->setToolTipsVisible(true);
  submenu->addAction(m_createOrderAction);

  ctx.exec(Ui()->dataView->viewport()->mapToGlobal(p));
}

void OrderDataViewer::openCreateOrderDialog() {
  OrderDialog od;
  connect(&od, &QDialog::accepted, this, &DataViewer::refresh);
  connect(&od, &OrderDialog::orderCreated, this, &OrderDataViewer::orderCreated);
  od.exec();
}