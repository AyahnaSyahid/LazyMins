#pragma once

#include "../realtimedatawidget.h"
#include "dashboardmodel.h"

namespace Ui {
  class SummaryWidget;
};

class SummaryWidget : public RealTimeDataWidget {
    Q_OBJECT

public:
  explicit SummaryWidget(QWidget *parent = nullptr);
  ~SummaryWidget();
  void addItem(const DashboardItem& item);
  
public slots:
  void reloadModelData(const QStringList& tables) override;
  
  
private:
  Ui::SummaryWidget *ui;
  DashboardModel *model;
};