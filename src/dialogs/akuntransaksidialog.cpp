#include "akuntransaksidialog.h"

#include <QMessageBox>

#include "src/customs/buttonguard.h"
#include "src/managers/managers.h"
#include "ui_akuntransaksidialog.h"

AkunTransaksiDialog::AkunTransaksiDialog(QWidget* parent)
    : ui(new Ui::AkunTransaksiDialog), FormDialog(parent) {
  ui->setupUi(this);
}

AkunTransaksiDialog::~AkunTransaksiDialog() { delete ui; }

void AkunTransaksiDialog::setupFields() {
  setFields({{ui->kodeLineEdit, "kode"},
             {ui->namaLineEdit, "nama"},
             {ui->penyediaLineEdit, "nama_bank"},
             {ui->rekeningLineEdit, "nomor_rekening"},
             {ui->atasNamaLineEdit, "atas_nama"},
             {ui->plainTextEdit, "description"}});
}

void AkunTransaksiDialog::setupBoundFields() {
  // FIX Bug 8: sertakan defaultValue agar clearFields() bisa reset ComboBox
  addBoundField(
      "tipe", [this]() { return ui->tipeComboBox->currentText(); },
      [this](const QVariant& va) {
        ui->tipeComboBox->setCurrentText(va.toString());
      },
      ui->tipeComboBox->itemText(0)  // default = item pertama
  );

  addBoundField(
      "is_active",
      [this]() { return ui->aktifComboBox->currentText() == "Ya" ? 1 : 0; },
      [this](const QVariant& va) {
        ui->aktifComboBox->setCurrentText(va.toInt() == 1 ? "Ya" : "Tidak");
      },
      0  // default = "Tidak"
  );
}

bool AkunTransaksiDialog::isInputAcceptable() const {
  QStringList errs;
  if (ui->kodeLineEdit->text().isEmpty()) errs << "- Field Kode";
  if (ui->namaLineEdit->text().isEmpty()) errs << "- Field Nama";
  if (ui->penyediaLineEdit->text().isEmpty()) errs << "- Field Nama Bank";
  if (ui->rekeningLineEdit->text().isEmpty()) errs << "- Field Nomor Rekening";
  if (ui->atasNamaLineEdit->text().isEmpty()) errs << "- Field Atas Nama";
  // if (ui->plainTextEdit->toPlainText().isEmpty())  errs << "- Field
  // Deskripsi";

  if (!errs.isEmpty()) {
    QMessageBox::warning(
        // FIX kecil: gunakan `this` bukan nullptr agar dialog modal
        // ter-parent dengan benar ke jendela ini
        const_cast<AkunTransaksiDialog*>(this), "Periksa Input",
        QString("Pastikan anda sudah mengisi semua field yang diminta:\n%1")
            .arg(errs.join("\n")));
    return false;
  }
  return true;
}

void AkunTransaksiDialog::on_simpanButton_clicked() {
  ButtonGuard guard(ui->simpanButton);
  if (!isInputAcceptable()) return;
  accept();
}

bool AkunTransaksiDialog::onSave(const QVariantMap& param) {
  if (param.isEmpty()) return false;
  AkunTransaksiManager mgr;
  if (isCreateMode()) {
    auto opt = mgr.create(param);
    if (!opt.has_value()) {
      qWarning() << "AkunTransaksiDialog: gagal membuat akun_transaksi:"
                 << mgr.errorString();
      return false;
    }
  } else {
    auto rc = originalRecord();
    // FIX Bug 6: pesan log "memperbarui", bukan "membuat"
    if (!mgr.update(rc.value("id").toInt(), param)) {
      qWarning() << "AkunTransaksiDialog: gagal memperbarui akun_transaksi:"
                 << mgr.errorString();
      return false;
    }
  }
  return true;
}
