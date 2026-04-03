#include "stockopnamedialog.h"
#include "ui_stockopnamedialog.h"

StockOpnameDialog::StockOpnameDialog(QWidget *parent) :
  ui(new Ui::StockOpnameDialog), m_product_id(-1), QDialog(parent) 
{
  ui->setupUi(this);
}

StockOpnameDialog::~StockOpnameDialog() {
  delete ui;
}

void StockOpnameDialog::setProductId(int pid)
{
  m_product_id = pid;
  
}
