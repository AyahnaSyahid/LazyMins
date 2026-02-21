#include "summarywidget.h"
#include "ui_summarywidget.h"
#include "../../databaseinterface.h"
#include "dashboarditemdelegate.h"

SummaryWidget::SummaryWidget(QWidget *parent) 
: ui(new Ui::SummaryWidget), model(new DashboardModel(this)), RealTimeDataWidget(parent)
{
  ui->setupUi(this);
  ui->listView->setModel(model);
  auto dlg = new DashboardItemDelegate(this);
  ui->listView->setItemDelegate(dlg);
}

SummaryWidget::~SummaryWidget()  { delete ui; }

void SummaryWidget::reloadModelData(const QStringList& tn)
{
  if ( tn.contains("invoices") ||
       tn.contains("payments") ) {
    auto &di = DatabaseInterface::instance();
    auto db = di.database();
  }
}

void SummaryWidget::addItem(const DashboardItem& d) {
  model->addItem(d);
}