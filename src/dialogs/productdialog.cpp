#include "productdialog.h"

#include <QMessageBox>
#include <QSqlQueryModel>

#include "src/customs/buttonguard.h"
#include "src/managers/managers.h"
#include "ui_productdialog.h"

ProductDialog::ProductDialog(QWidget* p)
    : ui(new Ui::ProductDialog), FormDialog(p) {
  ui->setupUi(this);
  auto unitModel = new QSqlQueryModel(this);
  unitModel->setQuery("SELECT DISTINCT unit FROM products ORDER BY unit ASC");
  ui->comboUnit->setModel(unitModel);
  ui->comboUnit->setCurrentText("");
}

ProductDialog::~ProductDialog() { delete ui; }

void ProductDialog::setupFields() {
  setFields({{ui->namaLineEdit, "name"},
             {ui->sKULineEdit, "sku"},
             {ui->descPlainTextEdit, "description"},
             {ui->costSpinBox, "cost_price"},
             {ui->minStock, "min_stock"},
             {ui->stokSpinBox, "stock"}});
}

void ProductDialog::setupBoundFields() {
  addBoundField(
      "category_id",
      [this]() { return ui->productCategoriesComboBox->currentId(); },
      [this](const QVariant& val) {
        ui->productCategoriesComboBox->setCurrentId(val.toInt());
      });
  addBoundField(
      "unit", [this]() { return ui->comboUnit->currentText(); },
      [this](const QVariant& val) {
        ui->productCategoriesComboBox->setCurrentText(val.toString());
      });
  addBoundField(
      "use_area", [this]() { return bool(ui->useAreaBox->currentIndex()); },
      [this](const QVariant& val) {
        ui->useAreaBox->setCurrentIndex(val.toInt());
      });
}

bool ProductDialog::onSave(const QVariantMap& map) {
  ProductManager pmg;
  auto crt = pmg.create(map);
  if (crt) {
    return true;
  }
  return false;
}

void ProductDialog::on_simpanButton_clicked() {
  ButtonGuard guard(ui->simpanButton);
  if (!isInputAcceptable()) return;
  accept();
}

void ProductDialog::onPrepareCreate() {
  ui->productCategoriesComboBox->blockSignals(true);
  ui->productCategoriesComboBox->setCurrentIndex(-1);
  ui->productCategoriesComboBox->blockSignals(false);
}

bool ProductDialog::isInputAcceptable() const {
  QStringList errs;
  if (ui->namaLineEdit->text().isEmpty()) errs << "- Nama harus diisi";
  if (ui->sKULineEdit->text().isEmpty()) errs << "- SKU harus diisi";
  if (ui->descPlainTextEdit->toPlainText().isEmpty())
    errs << "- Deskripsi harus diisi";
  if (ui->productCategoriesComboBox->currentIndex() < 0)
    errs << "- Kategori belum ditentukan";
  if (errs.size()) {
    QMessageBox::warning(
        nullptr, "Input belum lengkap",
        "Periksa kebutuhan input berikut terpenuhi:\n" + errs.join("\n"));
    return false;
  }
  return true;
}
