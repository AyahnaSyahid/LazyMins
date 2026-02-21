#pragma once

#include "../realtimedatawidget.h"

namespace Ui {
  class SummaryWidget;
};

class SummaryWidget : public RealTimeDataWidget
{
  Q_OBJECT
  
  public:
    explicit SummaryWidget(QWidget *p=nullptr);
    ~SummaryWidget();
  
  public slots:
    void reloadModelData(const QStringList& tn) override;
  
  private:
    Ui::SummaryWidget *ui;
};