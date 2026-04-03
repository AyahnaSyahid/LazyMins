#pragma once

#include <QDialog>
#include "src/managers/managers.h"

namespace Ui {
  class StockOpnameDialog;
}

class StockOpnameDialog : public QDialog
{
  
  Q_OBJECT
  public:
    explicit StockOpnameDialog(QWidget *p=nullptr);
    ~StockOpnameDialog();
    void setProductId(int pid);
  
  private:
    Ui::StockOpnameDialog *ui;
    int m_product_id;
    ProductManager productManager;
};