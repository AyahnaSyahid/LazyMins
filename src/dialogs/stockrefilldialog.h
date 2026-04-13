#pragma once

#include <QDialog>
#include "src/managers/productmanager.h"

namespace Ui {
  class StockRefillDialog;
}

class StockRefillDialog : public QDialog
{
  Q_OBJECT
  public:
    explicit StockRefillDialog(QWidget * =nullptr);
    ~StockRefillDialog();
    bool setProductId(int);

  private slots:
    void on_simpanButton_clicked();
  
  private:
    Ui::StockRefillDialog *ui;
    int m_productId = -1;
    ProductManager productManager;
};