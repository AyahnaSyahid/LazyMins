#include "orderpickerdialog.h"

#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTimer>

#include "src/managers/basemanager.h"
#include "ui_orderpickerdialog.h"

namespace {
class Delegate : public QStyledItemDelegate {
 public:
  Delegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}
  ~Delegate() = default;
  void initStyleOption(QStyleOptionViewItem* option,
                       const QModelIndex& index) const override {
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
}  // namespace

OrderPickerDialog::OrderPickerDialog(QWidget* parent)
    : QDialog(parent),
      ui(new Ui::OrderPickerDialog),
      model(new QSqlQueryModel(this)) {
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
  connect(ui->lineEdit, &QLineEdit::textChanged, delayer,
          [delayer]() { delayer->start(); });
  connect(this, &OrderPickerDialog::parameterChanged, this,
          &OrderPickerDialog::onParameterChanged);
}

OrderPickerDialog::~OrderPickerDialog() { delete ui; }

void OrderPickerDialog::setCustomerId(int id) {
  m_setMode = SetById;
  m_customer_id = id;
  emit parameterChanged();
}

void OrderPickerDialog::setCustomerName(const QString& name) {
  m_setMode = SetByName;
  m_customerName = name;
  emit parameterChanged();
}

void OrderPickerDialog::onParameterChanged() {
  if (m_setMode == NotSet) {
    model->setQuery("SELECT 0 WHERE 1=0"); // Empty query
    return;
  }

  QString baseQuery(R"--(
    SELECT id AS ID,
           customer_name AS Pelanggan,
           order_number AS Nomor,
           subtotal AS Subtotal,
           discount_amount AS Diskon,
           total_amount AS Total,
           date(order_date, 'localtime') AS Tanggal
      FROM orders
     %1
  ORDER BY order_date ASC )--");
  QString whereClause;
  if (m_setMode == SetById) {
    whereClause = "WHERE customer_id = :customer AND invoice_id IS NULL AND staging_status <> 'cancelled'";
  } else if (m_setMode == SetByName) {
    whereClause = "WHERE customer_name = :customer AND customer_id IS NULL AND invoice_id IS NULL AND staging_status <> 'cancelled'";
  }

  if (!m_filter_ids.isEmpty()) {
    QStringList ids;
    for (auto const& fid : m_filter_ids) ids << QString::number(fid);
    whereClause += QString(" AND id NOT IN ( %1)").arg(ids.join(", "));
  }

  baseQuery = baseQuery.arg(whereClause);
  QSqlQuery q(BaseManager::connection);

  q.prepare(baseQuery);
  if (m_setMode == SetById) {
    q.bindValue(":customer", m_customer_id);
  } else if (m_setMode == SetByName) {
    q.bindValue(":customer", m_customerName);
  }

  if (!q.exec())
    qDebug() << "OrderPickerDialog : exec failed ->" << q.lastError().text();

  model->setQuery(std::move(q));
  while (model->canFetchMore()) model->fetchMore();

  auto proxy = qobject_cast<QSortFilterProxyModel*>(ui->orderView->model());
  if (proxy && proxy->sourceModel() != model) proxy->setSourceModel(model);
  proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
  proxy->setFilterKeyColumn(-1);
  ui->orderView->resizeColumnsToContents();
}

void OrderPickerDialog::setFilterIds(const QList<int>& ids) {
  m_filter_ids = ids;
  emit parameterChanged();
}

void OrderPickerDialog::on_orderView_clicked(const QModelIndex& index) {
  if (!index.isValid()) {
    return;
  }
  auto proxy = findChild<QSortFilterProxyModel*>();
  if (!proxy) return;
  // Handle the selection of a order
  auto record = model->record(proxy->mapToSource(index).row());
  emit ordersPicked(QList<int>{record.value(0).toInt()});
  accept();  // Close the dialog after selection
}