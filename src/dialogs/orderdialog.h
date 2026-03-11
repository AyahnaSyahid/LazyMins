#pragma once

namespace Ui
{
  class OrderDialog;
}

#include "formdialog.h"
#include "src/managers/managers.h"
#include "src/models/orderitemeditormodel.h"

class OrderDialog : public FormDialog
{
  Q_OBJECT

public:
  explicit OrderDialog(QWidget * = nullptr);
  ~OrderDialog();
  void prepareModify(const QSqlRecord &r);
  QVariantMap collect() const override;
  
protected:
  void setupFields() override;
  void setupBoundFields() override;
  bool onSave(const QVariantMap &) override;
  void onPrepareCreate() override;
  void onPrepareModify(const QSqlRecord &orderRecord);
  void onOrderItemDialogAccepted(const QVariantMap& ss);


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