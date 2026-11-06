#include "konsumenrankviewer.h"

#include <QDateEdit>
#include <QDebug>
#include <QHeaderView>
#include <QLabel>
#include <QScrollBar>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTimer>
#include <QVBoxLayout>
#include <QValidator>

#include "src/managers/basemanager.h"

class KRDelegate : public QStyledItemDelegate {
 public:
  KRDelegate(QObject* parent = nullptr) {}
  ~KRDelegate() {}

 protected:
  void initStyleOption(QStyleOptionViewItem* opt,
                       const QModelIndex& ix) const override {
    QStyledItemDelegate::initStyleOption(opt, ix);
    if (ix.column() == 2 or ix.column() == 3) {
      opt->text = QLocale().toString(ix.data().toInt());
      if (ix.column() == 3) {
        opt->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
      } else {
        opt->displayAlignment = Qt::AlignCenter;
      }
    }
  }
};

KonsumenRankViewer::KonsumenRankViewer(QWidget* parent)
    : QWidget(parent),
      filterTimer(new QTimer(this)),
      startDateEdit(new QDateEdit(this)),
      endDateEdit(new QDateEdit(this)),
      _model(new QSqlQueryModel(this)) {
  // Inisialisasi UI
  startDateEdit->setDisplayFormat("yyyy-MM-dd");
  endDateEdit->setDisplayFormat("yyyy-MM-dd");
  for (auto sp : QList<QDateEdit*>{startDateEdit, endDateEdit}) {
    sp->setButtonSymbols(QAbstractSpinBox::NoButtons);
    sp->setAlignment(Qt::AlignCenter);
    sp->setCurrentSection(QDateTimeEdit::YearSection);
  }
  auto layout = new QVBoxLayout(this);
  auto view = new QTableView(this);
  auto filterLayout = new QHBoxLayout();
  auto l1 = new QLabel("Filter Tanggal");
  filterTimer->setSingleShot(true);
  filterTimer->setInterval(300);
  filterLayout->addWidget(l1);
  filterLayout->addWidget(startDateEdit);
  filterLayout->addSpacerItem(new QSpacerItem(20, 0));
  filterLayout->addWidget(endDateEdit);

  connect(filterTimer, &QTimer::timeout, this, &KonsumenRankViewer::fetchData);
  auto proxy = new QSortFilterProxyModel(this);
  proxy->setSourceModel(_model);
  view->setModel(proxy);
  view->setItemDelegate(new KRDelegate(this));
  view->setEditTriggers(QAbstractItemView::NoEditTriggers);
  view->setSelectionBehavior(QAbstractItemView::SelectRows);
  view->setSelectionMode(QAbstractItemView::SingleSelection);
  view->verticalHeader()->hide();
  view->verticalHeader()->setMinimumSectionSize(18);
  view->verticalHeader()->setDefaultSectionSize(18);
  view->setSortingEnabled(true);

  layout->addLayout(filterLayout);
  layout->addWidget(view, 1);

  auto refreshShortcut = new QShortcut(QKeySequence(Qt::Key_F5), this);
  connect(refreshShortcut, &QShortcut::activated, this,
          &KonsumenRankViewer::fetchData);

  initDateFilter();
  connect(endDateEdit, &QDateEdit::dateChanged, this,
          &KonsumenRankViewer::handleFilterEdit);
  connect(startDateEdit, &QDateEdit::dateChanged, this,
          &KonsumenRankViewer::handleFilterEdit);

  fetchData();
}

KonsumenRankViewer::~KonsumenRankViewer() {
  // _model akan dihapus otomatis karena parent-nya adalah 'this'
}

void KonsumenRankViewer::handleFilterEdit() {
  auto dateEdit = qobject_cast<QDateEdit*>(sender());
  if (!dateEdit) return;  // nothing happen
  if (dateEdit == startDateEdit) {
    if (startDateEdit->date() > endDateEdit->date()) {
      startDateEdit->blockSignals(true);
      startDateEdit->setDate(endDateEdit->date());
      startDateEdit->blockSignals(false);
    } else {
      filterTimer->start();
    }
  } else {
    if (endDateEdit->date() < startDateEdit->date()) {
      endDateEdit->blockSignals(true);
      endDateEdit->setDate(startDateEdit->date());
      endDateEdit->blockSignals(false);
    } else {
      filterTimer->start();
    }
  }
}

void KonsumenRankViewer::initDateFilter() {
  // fetch start and end date from database
  auto q = BaseManager::baseQuery();
  q.exec(R"-(
        SELECT MIN(DATE(payments.payment_date, 'localtime') ) AS BEGINING,
            MAX(DATE(payments.payment_date, 'localtime') ) AS ENDING
        FROM payments;
        )-");
  q.next();
  startDateEdit->setDate(q.value(0).toDate());
  endDateEdit->setDate(q.value(1).toDate());
}

void KonsumenRankViewer::fetchData() {
  // Query untuk meranking konsumen berdasarkan total nilai belanja/invoice
  // Sesuaikan kolom dan logika bisnis (misal: hanya hitung invoice yang sudah
  // lunas)
  const QString queryText = R"-(
        SELECT 
            k.nama_lengkap AS "Nama Konsumen",
            k.customer_code AS "Kode",
            COUNT(i.id) AS "Invoice",
            SUM(p.amount) AS "Total"
        FROM konsumen k
        JOIN invoices i ON k.id = i.customer_id
        JOIN payments p ON i.id = p.invoice_id
        WHERE p.verification_status = 'verified' AND p.payment_date BETWEEN :startDate AND :endDate
        GROUP BY k.id
        ORDER BY "Total" DESC
        LIMIT 100
    )-";

  QSqlQuery q(BaseManager::connection);
  q.prepare(queryText);
  q.bindValue(":startDate", startDateEdit->date().toString("yyyy-MM-dd"));
  q.bindValue(":endDate", endDateEdit->date().toString("yyyy-MM-dd"));
  q.exec();
  _model->setQuery(std::move(q));

  if (_model->lastError().isValid()) {
    qDebug() << "KonsumenRankViewer Error:" << _model->lastError().text();
  } else {
    onDataReady();
  }
}

void KonsumenRankViewer::onDataReady() {
  // Logika tambahan setelah data dimuat, seperti resize kolom
  // Kita perlu mencari QTableView di layout atau menyimpannya sebagai member
  auto view = findChild<QTableView*>();
  if (view) {
    view->horizontalHeader()->setStretchLastSection(false);
    view->resizeColumnsToContents();

    auto min_dialog_width = view->horizontalHeader()->length() +
                            // (layout()->contentsMargins().right() * 2);
                            view->verticalScrollBar()->width();
    setMinimumWidth(min_dialog_width);

    view->horizontalHeader()->setStretchLastSection(true);
    view->sortByColumn(3, Qt::DescendingOrder);
  }
}
