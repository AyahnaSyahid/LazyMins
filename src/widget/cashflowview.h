#pragma once

#include "realtimedatawidget.h"
#include <QLabel>
#include <QTableView>
#include <QSqlQueryModel>

class CashFlowDailyView : public RealTimeDataWidget
{
  Q_OBJECT
  public:
    CashFlowDailyView(QWidget *p=nullptr);
    ~CashFlowDailyView();
    void reloadModelData(const QStringList& tables) override;

  private slots:
    void initLabelData();
    
  private:
    
    QTableView *view;
    QSqlQueryModel *model;
    QLabel *outLabel, *inLabel, *balLabel;
};