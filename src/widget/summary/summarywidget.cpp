#include "summarywidget.h"
#include "ui_summarywidget.h"
#include "src/databaseinterface.h"
#include <QTimer>

SummaryWidget::SummaryWidget(QWidget *p) :
  ui(new Ui::SummaryWidget), RealTimeDataWidget(p)
{
  ui->setupUi(this);
  QTimer::singleShot(0, [this]() { reloadModelData({"invoices"}); });
  connect(&DatabaseInterface::instance(), &DatabaseInterface::tableUpdate, this, &SummaryWidget::reloadModelData);
};

SummaryWidget::~SummaryWidget() {
  delete ui;
}

void SummaryWidget::reloadModelData(const QStringList &tables) {
  if(tables.contains("invoices") || tables.contains("payments")) {    
    auto db = DatabaseInterface::instance().database();
    QSqlQuery q("SELECT * FROM omsetHarian", db);
    if (q.next()) {
      ui->value1->setText(QString("Rp. %L1").arg(q.value(0).toInt()));
      ui->frame->hide();
    }
  }
}