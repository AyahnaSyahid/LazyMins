#include "cashflowview.h"
#include "src/databaseinterface.h"

#include <QSqlQuery>
#include <QVBoxLayout>
#include <QHBoxLayout>

CashFlowDailyView::CashFlowDailyView(QWidget *p) : 
RealTimeDataWidget(p) {
  view = new QTableView(this);
  model = new QSqlQueryModel(this);
  outLabel = new QLabel(this);
  inLabel = new QLabel(this);
  balLabel = new QLabel(this);
  
  QSqlQuery q(DatabaseInterface::instance().database());
  q.exec(R"--(
    SELECT admin AS Admin,
           tipe AS Jenis,
           amount AS Jumlah,
           detail AS Detail
      FROM catatan_keluar_masuk_cash
     WHERE date(created_at, 'localtime') = date(CURRENT_DATE, 'localtime')
  )--");
  model->setQuery(std::move(q));
  view->setModel(model);
  
  auto ml = new QVBoxLayout(this);
  auto lab = new QHBoxLayout();
  lab->addWidget(inLabel);
  lab->addWidget(outLabel);
  lab->addWidget(balLabel);
  
  ml->addWidget(view, 1);
  ml->addLayout(lab);
  
  initLabelData();
};

CashFlowDailyView::~CashFlowDailyView() {}

void CashFlowDailyView::initLabelData() {
  int in = 0, out = 0, bal = 0;
  QString tipe;
  for(int i=0; i<model->rowCount(); ++i) {
    if(model->index(i, 1).data().toString().toLower() == "pengeluaran") {
      out += model->index(i, 2).data().toInt();
    } else {
      in += model->index(i, 2).data().toInt();
    }
  }
  bal = in - out;
  inLabel->setText(QString("IN %L1").arg(in));
  outLabel->setText(QString("OUT %L1").arg(out));
  balLabel->setText(QString("BLC %L1").arg(bal));
}

void CashFlowDailyView::reloadModelData(const QStringList& tables) {
  if (tables.contains("catatan_keluar_masuk_cash")) {
    model->setQuery(model->query().lastQuery(), DatabaseInterface::instance().database());
  }
}