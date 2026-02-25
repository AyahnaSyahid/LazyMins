#include "edituserdialog.h"
#include "ui_edituserdialog.h"

#include "src/dialogs/revokepassworddialog.h"
#include "src/dialogs/edituserlogininfodialog.h"
#include "src/managers/adminmanager.h"

#include <QTimer>
#include <QMessageBox>

EditUserDialog::EditUserDialog(const QString& name, QWidget *p):
ui(new Ui::EditUserDialog), QDialog(p)
{
  ui->setupUi(this);
  setWindowTitle("Edit User : " + name);
  setProperty("editUsername", name);
  AdminManager aa;
  auto opt = aa.getRecord(name);
  if (opt) {
    auto ur = *opt;
    info.id           = ur.value("id").toInt();
    info.role_id      = ur.value("role_id").toInt();
    info.username     = ur.value("username").toString();
    info.nama_lengkap = ur.value("nama_lengkap").toString();
    info.email        = ur.value("email").toString();
    info.nomor_telp   = ur.value("nomor_telp").toString();
  } else {
    // close dialog
    QTimer::singleShot(0, this, &EditUserDialog::userNotFound);
  }
}

EditUserDialog::~EditUserDialog() { delete ui; }

void EditUserDialog::on_ubahButton_clicked() {
  RevokePasswordDialog rpd(this);
  if(QDialog::Accepted != rpd.exec()) return;
  EditUserLoginInfoDialog eulid(property("editUsername").toString(), this);
  if(eulid.exec() == QDialog::Accepted) {
    auto newName = eulid.currentUsername();
    info.username = newName;
    setWindowTitle("Edit User : " + newName);
    emit userdataChanged(newName);
  }
}

void EditUserDialog::on_simpanButton_clicked() {
  if (ui->fullnameEdit->text().isEmpty() || ui->fullnameEdit->text().size() < 6) {
    QMessageBox::warning(this, "Peringatan", "Nama Lengkap tidak boleh kosong.\nminimal 6 karakter.");
    return ;
  }
  QVariantMap updateParams;
  if(ui->fullnameEdit->isModified() && ui->fullnameEdit->text() != info.nama_lengkap) {
    updateParams["nama_lengkap"] = ui->fullnameEdit->text().trimmed();
  }
  if(ui->phoneEdit->isModified() && ui->phoneEdit->text() != info.nomor_telp) {
    updateParams["nomor_telp"] = ui->phoneEdit->text().trimmed();
  }
  if(ui->emailEdit->isModified() && ui->emailEdit->text() != info.nomor_telp) {
    updateParams["email"] = ui->emailEdit->text().trimmed();
  }
  
}

void EditUserDialog::userNotFound() {
  QMessageBox::critical(this, "Kesalahan", "Username tidak dikenali");
  reject();
}
