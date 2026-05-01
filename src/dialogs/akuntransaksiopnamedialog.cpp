#include "akuntransaksiopnamedialog.h"

#include <QMessageBox>

#include "src/managers/akuntransaksimanager.h"
#include "ui_akuntransaksiopnamedialog.h"

AkunTransaksiOpnameDialog::AkunTransaksiOpnameDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::AkunTransaksiOpnameDialog) {
  ui->setupUi(this);
}

AkunTransaksiOpnameDialog::~AkunTransaksiOpnameDialog() { delete ui; }

bool AkunTransaksiOpnameDialog::prepareOpname(int akunId) {
  AkunTransaksiManager mgr;
  auto optAcc = mgr.getById(akunId);
  if (!optAcc) return false;

  m_record = optAcc.value();
  ui->nameLabel->setText(m_record.value("name").toString());
  ui->currentBox->setValue(m_record.value("saldo").toInt());
  ui->realBox->setValue(ui->currentBox->value());
  ui->adjustBox->setValue(ui->realBox->value() - ui->currentBox->value());
  return true;
}

void AkunTransaksiOpnameDialog::on_realBox_valueChanged(int a) {
  ui->adjustBox->setValue(a - ui->currentBox->value());
}

void AkunTransaksiOpnameDialog::on_simpanButton_clicked() {
  ui->simpanButton->setEnabled(false);
  if (ui->adjustBox->value() == 0) {
    accept();
  } else {
    if (ui->notesEdit->toPlainText().simplified().isEmpty()) {
      QMessageBox::warning(
          this, "Berikan Catatan",
          "Opname / Adjustment harus disertai dengan alasannya");
      ui->simpanButton->setEnabled(true);
      return;
    }
    AkunTransaksiManager mgr;
    if (!mgr.opname(m_record.value("id").toInt(), ui->realBox->value(),
                    ui->notesEdit->toPlainText())) {
      QMessageBox::warning(this, "Gagal melakukan Opname", mgr.errorString());
      ui->simpanButton->setEnabled(true);
      return;
    }
    accept();
  }
}
