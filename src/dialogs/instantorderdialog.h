#pragma once

#include "src/managers/managers.h"
#include "src/models/ordermodel.h"

#include <QDialog>

namespace Ui {
  class InstantOrderDialog;
}

// InstantOrderDialog menangani order instant
class InstantOrderDialog : public QDialog
{
  Q_OBJECT

public:
  explicit InstantOrderDialog(QWidget *parent = nullptr);
  ~InstantOrderDialog();


private slots:
  void on_pilihButton_clicked();
  void on_bayarButton_clicked();
  void on_orderListView_customContextMenuRequested(const QPoint&);
  void addItem(const OrderItem &item) { omod.addItem(item); }
  void recalculate();

private:
  bool checkInput();
  int  calculatedSubtotal() const;

private:
  struct KonsumenSet {
    int id = -1, 
        price_level = 1;
    QString name = "",
            phone = "";
  } customerSet;

  Ui::InstantOrderDialog *ui;
  OrderManager oman;
  OrderModel omod;
  InvoiceManager m_invm;
};