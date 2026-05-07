#include "stockopnamedialog.h"

#include <QMessageBox>

#include "src/customs/buttonguard.h"
#include "src/managers/helpers.h"
#include "ui_stockopnamedialog.h"

StockOpnameDialog::StockOpnameDialog(QWidget* parent)
    : ui(new Ui::StockOpnameDialog),
      m_product_id(-1),
      m_currentStock(0),
      m_needed(0),
      QDialog(parent) {
  ui->setupUi(this);
}

StockOpnameDialog::~StockOpnameDialog() { delete ui; }

void StockOpnameDialog::setProductId(int pid) {
  m_product_id = pid;
  auto prod = *productManager.getById(pid);
  m_currentStock = prod.value("stock").toDouble();
  ui->labelSku->setText(prod.value("sku").toString());
  ui->labelNama->setText(prod.value("name").toString());
  ui->labelDeskripsi->setText(prod.value("description").toString());
  ui->dataSpinBox->setValue(m_currentStock);
  ui->currentSpinBox->setValue(m_currentStock);
}

void StockOpnameDialog::on_currentSpinBox_valueChanged(qreal cn) {
  m_needed = cn - ui->dataSpinBox->value();
  ui->lineEdit->setText(locale().toString(m_needed));
}

void StockOpnameDialog::on_simpanButton_clicked() {
  ButtonGuard guard(ui->simpanButton);
  if (m_needed == 0) {
    reject();
    return;
  }
  QString notes(ui->notesEdit->toPlainText().simplified());
  if (notes.isEmpty()) {
    QMessageBox::warning(this, "Berikan Catatan",
                         "Opname / Adjustment harus disertai dengan alasannya");
    return;
  }
  auto res = DBOperationHelper::adjustProductStock(
      m_product_id, ui->currentSpinBox->value(), notes);
  if (!res.ok) {
    QMessageBox::warning(this, "Gagal melakukan Opname", res.error);
    return;
  }
  accept();
}