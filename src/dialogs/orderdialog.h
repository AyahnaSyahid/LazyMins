#pragma once

namespace Ui
{
  class OrderDialog;
}

#include "src/managers/managers.h"
#include "src/models/ordermodel.h"
#include <QDialog>
class OrderDialog : public QDialog
{
  Q_OBJECT

public:
  explicit OrderDialog(QWidget * = nullptr);
  ~OrderDialog();

private slots:
  void addOrderItem(const OrderItem& oi);
  void on_simpanButton_clicked();
  void on_tambahItem_triggered();
  void on_cariButton_clicked();
  void updateCalculation();
  void on_orderItemList_customContextMenuRequested(const QPoint &pos);
  void on_diskonDoubleSpinBox_valueChanged(double arg1);
  void on_diskonRpSpinBox_valueChanged(int arg1);
  void setCustomer(const QSqlRecord&);
  void editOrderItemDialogFinished();

signals:
  void orderCreated(int id);

private:
  Ui::OrderDialog *ui;
  OrderModel *m_model;
  OrderManager oman;
  
  struct KonsumenSet {
    int id = -1;
    QString name;
  } customerSet {};

};