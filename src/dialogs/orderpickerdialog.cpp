#include "orderpickerdialog.h"
#include "ui_orderpickerdialog.h"
#include "src/managers/basemanager.h"
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QTimer>

namespace {
  class Delegate: public QStyledItemDelegate
  {
    public:
      Delegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
      ~Delegate() = default;
      void initStyleOption(QStyleOptionViewItem *option, const QModelIndex& index) const override {
        QStyledItemDelegate::initStyleOption(option, index);
        switch (index.column()) {
          case 0:
            option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            break;
          case 1:
          case 5:
            option->displayAlignment = Qt::AlignCenter;
            break;
          case 2:
          case 3:
          case 4:
            option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            option->text = QString("%L1").arg(index.data().toInt());
            break;
        }
      }
  };
}

OrderPickerDialog::OrderPickerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OrderPickerDialog),
    model(new QSqlQueryModel(this))
{
    ui->setupUi(this);
    auto proxy = new QSortFilterProxyModel(this);
    ui->orderView->setModel(proxy);
    
    ui->orderView->horizontalHeader()->setStretchLastSection(true);
    ui->orderView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->orderView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->orderView->verticalHeader()->setMinimumSectionSize(20);
    ui->orderView->verticalHeader()->setDefaultSectionSize(22);
    ui->orderView->setAlternatingRowColors(true);
    ui->orderView->setItemDelegate(new Delegate(this));
    auto delayer = new QTimer(this);
    delayer->setSingleShot(true);
    delayer->setInterval(500);
    
    connect(delayer, &QTimer::timeout, [this, proxy]() {
      proxy->setFilterFixedString(ui->lineEdit->text());
    });
    // proxy->setSourceModel(model);
    connect(ui->lineEdit, &QLineEdit::textChanged, delayer, [delayer](){delayer->start(); });
    connect(this, &OrderPickerDialog::parameterChanged, this, &OrderPickerDialog::onParameterChanged);
}

OrderPickerDialog::~OrderPickerDialog() { delete ui; }

void OrderPickerDialog::setCustomerId(int id)
{
  m_customer_id = id;
  emit parameterChanged();
}

void OrderPickerDialog::onParameterChanged() {
  
  QString baseQuery (R"--(
    SELECT id AS ID,
           order_number AS Nomor,
           subtotal AS Subtotal,
           discount_amount AS Diskon,
           total_amount AS Total,
           date(order_date, 'localtime') AS Tanggal
      FROM orders
     WHERE customer_id = :cust_id AND invoice_id IS NULL AND id NOT IN ( %1)
  ORDER BY order_date ASC )--");
  
  QStringList ids;
  for(auto const &fid : m_filter_ids) ids << QString::number(fid);
  
  baseQuery = baseQuery.arg(ids.join(", "));
  QSqlQuery q(BaseManager::connection);
  
  q.prepare(baseQuery);
  q.bindValue(":cust_id", m_customer_id);
  if (! q.exec() )  qDebug() << "OrderPickerDialog : exec failed ->" << q.lastError().text();
  
  model->setQuery(std::move(q));
  while(model->canFetchMore()) model->fetchMore();
  
  auto proxy = qobject_cast<QSortFilterProxyModel*>(ui->orderView->model());
  if (proxy && proxy->sourceModel() != model) proxy->setSourceModel(model);
  ui->orderView->resizeColumnsToContents();
}

void OrderPickerDialog::setFilterIds(const QList<int> &ids) {
  m_filter_ids = ids;
  emit parameterChanged();
}

void OrderPickerDialog::on_orderView_clicked(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }
    auto proxy = findChild<QSortFilterProxyModel*>();
    if(!proxy) return;
    // Handle the selection of a order
    auto record = model->record(proxy->mapToSource(index).row());
    emit ordersPicked(QList<int> {record.value(0).toInt()});
    accept(); // Close the dialog after selection    
}   