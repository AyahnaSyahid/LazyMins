#include "edituserlogininfodialog.h"
#include "ui_edituserlogininfodialog.h"

#include "src/managers/adminmanager.h"

#include <QMessageBox>

EditUserLoginInfoDialog::EditUserLoginInfoDialog(const QString& un, QWidget *p):
m_username(un), QDialog(p)
{
  ui->setupUi(this);
  ui->oldUserLabel->setText(un);
  ui->newUserEdit->setText(un);
}

EditUserLoginInfoDialog::~EditUserLoginInfoDialog() { delete ui; }

void EditUserLoginInfoDialog::on_simpanButton_clicked(){
  bool userChanges = false,
       passChanges = false;
  QString newUsername = ui->newUserEdit->text().trimmed(),
          newPass     = ui->newPassEdit->text();
  
  if(newUsername != m_username) {
    if (newUsername.isEmpty()) {
      QMessageBox::warning(this, "Peringatan", "Username tidak boleh kosong.");
      return ;
    }
    if (newUsername.size() < 5) {
      QMessageBox::warning(this, "Peringatan", "Username terlalu pendek, batas minimal 5 karakter.");
      return ;
    }
    userChanges = true;
  }
  
  if (!newPass.isEmpty()) {
    if (newPass.size() < 6) {
      QMessageBox::warning(this, "Peringatan", "Password terlalu pendek, batas minimal 6 karakter.");
      return ;  
    }
    passChanges = true;
  }
  
  if (!userChanges && !passChanges) {
    // tidak ada perubahan
    accept();
    return;
  }
  
  AdminManager am;
  if (userChanges && passChanges) {
    if (am.changeLoginInfo(m_username, newUsername, newPass)) {
      accept();
      return ;
    }
  }
  
  if (userChanges) {
    if (am.changeUsername(m_username, newUsername)) {
      m_username = newUsername;
      accept();
      return ;
    }
  }
  
  if (passChanges) {
    if (am.changePassword(m_username, newPass)) {
      accept();
      return ;
    }
  }

}