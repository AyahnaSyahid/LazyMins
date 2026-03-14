#pragma once

namespace Ui
{
  class OrderDialog;
}

#include "src/managers/managers.h"
#include "src/models/orderitemeditormodel.h"

class OrderDialog : public QDialog
{
  Q_OBJECT

public:
  explicit OrderDialog(QWidget * = nullptr);
  ~OrderDialog();
  
protected:

private slots:
  void on_simpanButton_clicked();
  void on_tambahItem_triggered();
  void on_cariButton_clicked();
  void updateCalculation();
  void on_orderItemView_customContextMenuRequested(const QPoint &pos);
  void on_diskonDoubleSpinBox_valueChanged(double arg1);
  void on_diskonRpSpinBox_valueChanged(int arg1);
  void on_pajakRpSpinBox_valueChanged(int arg1);

private:
  Ui::OrderDialog *ui;
  OrderItemEditorModel *emodel;
  OrderManager oman;
  
  struct KonsumenSet {
    int id;
    QString name;
  } customerSet {};

};