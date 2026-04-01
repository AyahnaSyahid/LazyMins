#include "productdialog.h"
#include "ui_productdialog.h"
#include "src/managers/managers.h"
#include <QMessageBox>
ProductDialog::ProductDialog(QWidget *p):
ui(new Ui::ProductDialog), FormDialog(p)
{
  ui->setupUi(this);
}

ProductDialog::~ProductDialog() { delete ui; }

void ProductDialog::setupFields() {
  setFields({
    { ui->namaLineEdit, "name"},
    { ui->sKULineEdit,  "sku"},
    { ui->descPlainTextEdit,  "description"},
    { ui->costSpinBox,  "cost_price"},
    { ui->minStock,  "min_stock"},
    { ui->stokSpinBox,  "stock"}
  });
}

void ProductDialog::setupBoundFields() {
  addBoundField("category_id",
    [this](){ return ui->productCategoriesComboBox->currentId(); },
    [this](const QVariant& val){ ui->productCategoriesComboBox->setCurrentId(val.toInt()); } );
  addBoundField("unit",
    [this](){ return ui->comboUnit->currentText(); },
    [this](const QVariant& val){ ui->productCategoriesComboBox->setCurrentText(val.toString()); } );
  addBoundField("use_area",
    [this]() { return bool(ui->useAreaBox->currentIndex()); },
    [this](const QVariant& val) { ui->useAreaBox->setCurrentIndex(val.toInt()); } );
}

bool ProductDialog::onSave(const QVariantMap& map) {
  ProductManager pmg;
  auto crt = pmg.create(map);
  if(crt) {
    return true;
  }
  return false;
}

void ProductDialog::on_simpanButton_clicked() {
  accept();
}