#include "edituserlogininfodialog.h"

#include <QMessageBox>

#include "src/customs/buttonguard.h"
#include "src/managers/adminmanager.h"
#include "ui_edituserlogininfodialog.h"

EditUserLoginInfoDialog::EditUserLoginInfoDialog(const QString& un, QWidget* p)
    : QDialog(p), m_username(un), ui(new Ui::EditUserLoginInfoDialog) {
  ui->setupUi(this);
  ui->oldUserLabel->setText(un);
  ui->newUserEdit->setText(un);
}

EditUserLoginInfoDialog::~EditUserLoginInfoDialog() { delete ui; }

void EditUserLoginInfoDialog::on_simpanButton_clicked() {
  ButtonGuard guard(ui->simpanButton);
  QString newUsername = ui->newUserEdit->text().trimmed();
  QString newPass = ui->newPassEdit->text();

  bool userChanges = (newUsername != m_username);
  bool passChanges = !newPass.isEmpty();

  // 1. Validasi Input
  if (userChanges) {
    if (newUsername.isEmpty()) {
      QMessageBox::warning(this, "Peringatan", "Username tidak boleh kosong.");
      return;
    }
    if (newUsername.size() < 5) {
      QMessageBox::warning(this, "Peringatan",
                           "Username terlalu pendek (min. 5 karakter).");
      return;
    }
  }

  if (passChanges && newPass.size() < 6) {
    QMessageBox::warning(this, "Peringatan",
                         "Password terlalu pendek (min. 6 karakter).");
    return;
  }

  if (!userChanges && !passChanges) {
    accept();
    return;
  }

  // 2. Eksekusi Perubahan
  AdminManager am;
  bool success = false;

  if (userChanges && passChanges) {
    success = am.changeLoginInfo(m_username, newUsername, newPass);
  } else if (userChanges) {
    success = am.changeUsername(m_username, newUsername);
  } else if (passChanges) {
    success = am.changePassword(m_username, newPass);
  }

  // 3. Finalisasi
  if (success) {
    if (userChanges) m_username = newUsername;  // Selalu update state internal
    accept();
  } else {
    QMessageBox::critical(this, "Error", "Gagal memperbarui data ke database.");
  }
}