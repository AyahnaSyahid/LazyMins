#include "akuntransaksiopnamedialog.h"

#include <QMessageBox>

#include "src/controllers/akuntransaksi.h"
#include "src/managers/akuntransaksimanager.h"
#include "ui_akuntransaksiopnamedialog.h"

AkunTransaksiOpnameDialog::AkunTransaksiOpnameDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::AkunTransaksiOpnameDialog) {
  ui->setupUi(this);
}

AkunTransaksiOpnameDialog::~AkunTransaksiOpnameDialog() { delete ui; }

bool AkunTransaksiOpnameDialog::prepareOpname(int akunId) {
  AkunTransaksiController ctr;
  auto data = ctr.getAccountData(akunId);
  if (data.isEmpty()) return false;
  m_data = data;
  ui->nameLabel->setText(m_data.value("name").toString());
  ui->currentBox->setValue(m_data.value("saldo").toInt());
  ui->realBox->setValue(ui->currentBox->value());
  ui->adjustBox->setValue(ui->realBox->value() - ui->currentBox->value());
  return true;
}

QVariantMap AkunTransaksiOpnameDialog::paramFromUi() const {
  return {{"notes", ui->notesEdit->toPlainText().simplified().trimmed()},
          {"real_saldo", ui->realBox->value()}};
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
    AkunTransaksiController ctr;
    QString err;
    if (!ctr.makeOpname(m_data.value("id").toInt(), paramFromUi(), &err)) {
      QMessageBox::warning(this, "Kesalahan", err);
      ui->simpanButton->setEnabled(true);
      return;
    }
    accept();
  }
}
