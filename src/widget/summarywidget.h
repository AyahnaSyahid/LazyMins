#pragma once

#include "realtimedatawidget.h"

class SummaryWidget : public RealTimeDataWidget {
    Q_OBJECT

public:
  explicit SummaryWidget(QWidget *parent = nullptr);
    
public slots:
  void reloadModelData(const QStringList& tables) override;

private:
  QWidget* createStatCard(const QString &title, const QString &value, const QString &subtext, const QString &color);    
  QWidget* m_topLevel;
  void initTopLevel();
};