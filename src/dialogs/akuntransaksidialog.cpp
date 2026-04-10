#include "akuntransaksidialog.h"
#include "ui_akuntransaksidialog.h"

#include "src/managers/managers.h"
#include <QMessageBox>

AkunTransaksiDialog::AkunTransaksiDialog(QWidget *parent):
ui(new Ui::AkunTransaksiDialog), FormDialog(parent)
{
  ui->setupUi(this);
}

AkunTransaksiDialog::~AkunTransaksiDialog() { delete ui; }

void AkunTransaksiDialog::setupFields() {
  setFields({
    {ui->kodeLineEdit,     "kode" },
    {ui->namaLineEdit,     "nama" },
    {ui->penyediaLineEdit, "nama_bank" },
    {ui->rekeningLineEdit, "nomor_rekening" },
    {ui->atasNamaLineEdit, "atas_nama" },
    {ui->plainTextEdit,    "description" }
  });
};

void AkunTransaksiDialog::setupBoundFields() {
  addBoundField( "tipe", 
    [this](){ return ui->tipeComboBox->currentText(); },
    [this](const QVariant& va){ ui->aktifComboBox->setCurrentText(va.toString()); }
    );

  addBoundField( "is_active", 
    [this](){ return ui->aktifComboBox->currentText() == "Ya" ? 1 : 0; },
    [this](const QVariant& va){ ui->aktifComboBox->setCurrentText(va.toInt() == 1 ? "Ya" : "Tidak"); }
    );
}

bool AkunTransaksiDialog::isInputAcceptable() const {
  QStringList errs;
  if(ui->kodeLineEdit->text().isEmpty()) errs << "- Field Kode";
  if(ui->namaLineEdit->text().isEmpty()) errs << "- Field Nama";
  if(ui->penyediaLineEdit->text().isEmpty()) errs << "- Field Nama Bank";
  if(ui->rekeningLineEdit->text().isEmpty()) errs << "- Field Nomor Rekening";
  if(ui->atasNamaLineEdit->text().isEmpty()) errs << "- Field Atas Nama";
  if(ui->plainTextEdit->toPlainText().isEmpty()) errs << "- Field Deskripsi";
  
  if (errs.size()) {
    QMessageBox::warning(nullptr, "Periksa Input", QString("Pastikan anda sudah mengisi semua field yang diminta:\n%1").arg(errs.join("\n")));
    return false;
  }
  return true;
}

void AkunTransaksiDialog::on_simpanButton_clicked() {
  accept();
}

bool AkunTransaksiDialog::onSave(const QVariantMap& param) {
  if (!param.size()) return false;
  AkunTransaksiManager mgr;
  if (isCreateMode()) {
    auto opt = mgr.create(param);
    if (!opt.has_value()) {
      auto err = mgr.errorString();
      qWarning() << "AkunTransaksiDialog: gagal membuat akun_transaksi:"
                 << err;
      return false;
    }
  } else {
    auto rc = originalRecord();
    if (!mgr.update(rc.value("id").toInt(), param)) {
      auto err = mgr.errorString();
      qWarning() << "AkunTransaksiDialog: gagal membuat akun_transaksi:"
                 << err;
      return false;
    }
  }
  return true;
}