#include "kategoriprodukdialog.h"

#include <QMessageBox>

#include "src/customs/buttonguard.h"
#include "ui_kategoriprodukdialog.h"

KategoriProdukDialog::KategoriProdukDialog(QWidget* p)
    : ui(new Ui::KategoriProdukDialog), m_id(-1), QDialog(p) {
  ui->setupUi(this);
}

KategoriProdukDialog::~KategoriProdukDialog() { delete ui; }

void KategoriProdukDialog::prepareCreate() {
  ui->IDLabel->setText("Kategori Baru");
  ui->statusComboBox->setCurrentIndex(1);
}

void KategoriProdukDialog::prepareModify(int _id) {
  auto opt_kp = pcm.getById(_id);
  if (!opt_kp.has_value() || _id < 1) {
    QMessageBox::information(
        this, "Kesalahan",
        QString("Tidak ditemukan data kategori dengan id \"%1\"\nmode beralih "
                "ke pembuatan kategori baru")
            .arg(_id));
    prepareCreate();
    return;
  }
  auto kp = *opt_kp;
  ui->IDLabel->setText(QString::number(_id));
  ui->statusComboBox->setCurrentIndex(kp.value("is_active").toBool() ? 1 : 0);
  ui->nameLineEdit->setText(kp.value("category_name").toString());
  ui->deskripsiPlainText->setPlainText(kp.value("description").toString());
}

bool KategoriProdukDialog::isInputAcceptable() const {
  QStringList errs;
  if (ui->nameLineEdit->text().isEmpty()) errs << "- Field Nama";
  if (ui->deskripsiPlainText->toPlainText().isEmpty())
    errs << "- Field Deskripsi";

  if (errs.size()) {
    QMessageBox::warning(
        nullptr, "Input belum lengkap",
        "Periksa kebutuhan input berikut terpenuhi:\n" + errs.join("\n"));
    return false;
  }
  return true;
}

void KategoriProdukDialog::on_simpanButton_clicked() {
  ButtonGuard guard(ui->simpanButton);
  if (!isInputAcceptable()) return;

  QMap<QString, QString> pairs{
      {"category_name", ui->nameLineEdit->text()},
      {"description", ui->deskripsiPlainText->toPlainText()},
  };

  if (m_id < 1) {
    // Create Mode
    auto opt_cr = pcm.create({{"category_name", pairs["category_name"]},
                              {"description", pairs["description"]},
                              {"created_at", QDateTime::currentDateTimeUtc()},
                              {"updated_at", QDateTime::currentDateTimeUtc()}});
    if (opt_cr.has_value()) {
      accept();
      return;
    }
    QMessageBox::information(this, "Input data gagal", pcm.errorString());
  } else {
    // Modify
    bool ok =
        pcm.update(m_id, {{"category_name", pairs["category_name"]},
                          {"description", pairs["description"]},
                          {"updated_at", QDateTime::currentDateTimeUtc()}});

    if (ok) {
      accept();
      return;
    }
    QMessageBox::information(this, "Update data gagal", pcm.errorString());
  }
}