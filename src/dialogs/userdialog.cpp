#include "userdialog.h"
#include "ui_userdialog.h"

#include "fieldmap.h"

#include <QMessageBox>
#include <QTimer>
#include "src/utils/sessionmanager.h"
#include "src/managers/adminmanager.h"

namespace
{
    // close using QTimer jika SessionManager tidak memiliki user aktif atau valid
    void hasValidUserOrClose(QDialog *w, const QString &errorMsg = "User tidak valid. Dialog akan ditutup.")
    {
        AdminManager adminManager;
        auto user = SessionManager::instance().currentUser();
        if (!SessionManager::instance().currentUser())
        {
            QMessageBox::warning(w, "User Tidak Valid", errorMsg);
            QTimer::singleShot(0, w, &QDialog::reject);
            return;
        }
        
        if (!user || !adminManager.userHasRole(user->value("id").toInt(), "super_admin"))
        {
            QMessageBox::warning(w, "Akses Ditolak", errorMsg);
            QTimer::singleShot(0, w, &QDialog::reject);
            return;
        }
        
    }
}

UserDialog::UserDialog(QWidget *parent) : ui(new Ui::UserDialog),
                                          FormDialog(parent)
{
    ui->setupUi(this);
}

UserDialog::~UserDialog()
{
    delete ui;
}

void UserDialog::setupFields()
{
    setFields({{ui->fullnameEdit, "nama_lengkap"},
               {ui->phoneEdit, "nomor_telp"},
               {ui->emailEdit, "email"},
               {ui->usernameEdit, "username"},
               {ui->passwordEdit1, "literal_password"}});
}

bool UserDialog::isInputAcceptable() const
{
  QStringList err;
  if (ui->fullnameEdit->text().trimmed().isEmpty()) err  << " - Nama lengkap";
  if (ui->phoneEdit->text().trimmed().isEmpty()) err     << " - Nomor Telepon";
  if (ui->emailEdit->text().trimmed().isEmpty()) err     << " - Email";
  if (ui->usernameEdit->text().trimmed().isEmpty()) err  << " - Username";
  if (ui->passwordEdit1->text().trimmed().isEmpty()) err << " - Password";
  
  auto pass1 = ui->passwordEdit1->text();
  auto pass2 = ui->passwordEdit2->text();
  
  if (pass1 != pass2) err << "- Password & Verifikasi Harus sama";
  
  if (err.size()) {
    QMessageBox::warning(nullptr, "Input Ditolak", QString("Pastikan kriteria berikut terpenuhi:\n%1").arg(err.join("\n")));
    return false;
  }
  return true;
}

bool UserDialog::onSave(const QVariantMap &changes)
{
  
  if (isCreateMode()) {
      auto opt = m_adminManager.create(changes);
      if (!opt.has_value()) {
        QMessageBox::warning(this, "Tidak dapat menyimpan", "Error: \n" + m_adminManager.errorString());
        return false;
      }
  } else {
      if (!m_adminManager.update(originalRecord().value("id").toInt(), changes)) {
        QMessageBox::warning(this, "Tidak dapat menyimpan", "Error: \n" + m_adminManager.errorString());
        return false;
      };
  }
  return true;
}

void UserDialog::on_simpanButton_clicked()
{
  if (!isInputAcceptable()) return;
  accept();
}