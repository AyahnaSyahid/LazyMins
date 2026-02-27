#include "productdialog.h"
#include "ui_productdialog.h"

ProductDialog::ProductDialog(QWidget *p):
ui(new Ui::ProductDialog), QDialog(p)
{
  ui->setupUi(this);
}

ProductDialog::~ProductDialog() { delete ui; }

void ProductDialog::setupFields() {
  setFields({
    { ui->namaLineEdit, "name"},
    { ui->sKULineEdit,  "sku"},
    { ui->descPlainTextEdit,  "description"},
    { ui->descPlainTextEdit,  "description"},
    
  });
}