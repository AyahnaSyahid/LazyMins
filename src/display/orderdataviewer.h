#pragma once

#include "dataviewer.h"

class QAction;
class OrderDataViewer : public DataViewer
{
  Q_OBJECT

public:
  explicit OrderDataViewer(QWidget *p=nullptr);
  ~OrderDataViewer();
  const QAction *createOrderAction() const { return m_createOrderAction; };

public slots:
  void openCreateOrderDialog();


private slots:
  void on_dataView_customContextMenuRequested(const QPoint& p);
  
signals:
  void orderCreated(int id);
  
private:
  QAction *m_createOrderAction;
};