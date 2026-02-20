#include "cashflowview.h"
#include "src/databaseinterface.h"

#include <QSqlQuery>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QStyledItemDelegate>

namespace {
  class NumberDelegate : public QStyledItemDelegate
  {
    public:
      using QStyledItemDelegate::QStyledItemDelegate;
      QString displayText(const QVariant& vv, const QLocale& ll) const {
        return QString("%L1").arg(vv.toLongLong());
      }
    protected:
      void initStyleOption(QStyleOptionViewItem *opt, const QModelIndex& mi) const {
        QStyledItemDelegate::initStyleOption(opt, mi);
        opt->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
      }
      
  };
}

CashFlowDailyView::CashFlowDailyView(QWidget *p) : 
RealTimeDataWidget(p) {
  view = new QTableView(this);
  model = new QSqlQueryModel(this);
  outLabel = new QLabel(this);
  inLabel = new QLabel(this);
  balLabel = new QLabel(this);
  
  QSqlQuery q(DatabaseInterface::instance().database());
  q.exec(R"--(
    SELECT detail AS Detail,
           amount AS Jumlah,
           admin AS Admin,
           tipe AS Jenis
      FROM catatan_keluar_masuk_cash
     WHERE date(created_at, 'localtime') = date(CURRENT_DATE, 'localtime')
  )--");
  model->setQuery(std::move(q));
  
  // View style
  view->setModel(model);
  view->setAlternatingRowColors(true);
  view->setItemDelegateForColumn(1, new NumberDelegate(this));
  auto vh = view->verticalHeader();
  vh->setMinimumSectionSize(20);
  vh->setDefaultSectionSize(22);
  vh->hide();
  
  auto ml = new QVBoxLayout(this);
  auto lab = new QHBoxLayout();
  lab->addWidget(inLabel);
  lab->addWidget(outLabel);
  lab->addWidget(balLabel);
  
  ml->addWidget(view, 1);
  ml->addLayout(lab);
  
  initLabelData();
  connect(&DatabaseInterface::instance(), &DatabaseInterface::tableUpdate, this, &CashFlowDailyView::reloadModelData);
};

CashFlowDailyView::~CashFlowDailyView() {}

void CashFlowDailyView::initLabelData() {
  int in = 0, out = 0, bal = 0;
  QString tipe;
  for(int i=0; i<model->rowCount(); ++i) {
    if(model->index(i, 3).data(Qt::EditRole).toString().toLower() == "pengeluaran") {
      out += model->index(i, 1).data(Qt::EditRole).toInt();
    } else {
      in += model->index(i, 1).data(Qt::EditRole).toInt();
    }
  }
  bal = in - out;
  inLabel->setText(QString("Masuk %L1").arg(in, 9));
  outLabel->setText(QString("Keluar %L1").arg(out, 9));
  balLabel->setText(QString("Balance %L1").arg(bal, 9));
}

void CashFlowDailyView::reloadModelData(const QStringList& tables) {
  if (tables.contains("catatan_keluar_masuk_cash")) {
    qDebug() << "Reloading model";
    model->setQuery(model->query().lastQuery(), DatabaseInterface::instance().database());
  }
  initLabelData();
}