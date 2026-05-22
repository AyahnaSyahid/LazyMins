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
  void setOrderStatus(const QModelIndex& ix, const QString& status);
  void viewOrderItems(const QModelIndex& ix);
  void cancelOrder(const QModelIndex& ix);

private:
  QString orderStatus(const QModelIndex&) const;

signals:
  void orderCreated(int id);
  void stockChanged(int orderId);
  void createInvoiceRequested(int id);
  
private:
  QAction *m_createOrderAction;
};