#pragma once

#include "dataviewer.h"

class QAction;
class OrderDataViewer : public DataViewer
{
  Q_OBJECT

public:
  explicit OrderDataViewer(QWidget *p=nullptr);
  ~OrderDataViewer();
  
private slots:
  void on_dataView_customContextMenuRequested(const QPoint& p);
  
private:
  QAction *newOrderAction;
};