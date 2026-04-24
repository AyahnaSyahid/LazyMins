#include "edituserdialog.h"
#include "ui_edituserdialog.h"

#include "src/dialogs/revokepassworddialog.h"
#include "src/dialogs/edituserlogininfodialog.h"
#include "src/managers/adminmanager.h"

#include <QTimer>
#include <QMessageBox>

EditUserDialog::EditUserDialog(const QString& name, QWidget *p):
QDialog(p), ui(new Ui::EditUserDialog), info(), m_userLoaded(false)
{
  ui->setupUi(this);
  setUser(name);
}

EditUserDialog::~EditUserDialog() { delete ui; }

bool EditUserDialog::setUser(const QString& name) {
  AdminManager aa;
  auto opt = aa.getRecord(name);
  if (!opt.has_value()) return false;
  
  auto ur = *opt;
  info.id           = ur.value("id").toInt();
  info.role_id      = ur.value("role_id").toInt();
  info.username     = ur.value("username").toString();
  info.nama_lengkap = ur.value("nama_lengkap").toString();
  info.email        = ur.value("email").toString();
  info.nomor_telp   = ur.value("nomor_telp").toString();
  
  ui->usernameEdit->setText(info.username);
  ui->fullnameEdit->setText(info.nama_lengkap);
  ui->phoneEdit->setText(info.nomor_telp);
  ui->emailEdit->setText(info.email);
  ui->roleBox->setCurrentRoleId(info.role_id);
  

  setWindowTitle("Edit User : " + name);
  setProperty("editUsername", name);
  m_userLoaded = true;
  return true;
}

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
    QStringList errors;

    // 1. Validasi Nama Lengkap
    QString fullname = ui->fullnameEdit->text().trimmed();
    if (fullname.isEmpty()) {
        errors << "- Nama Lengkap tidak boleh kosong.";
    } else if (fullname.length() < 6) {
        errors << "- Nama Lengkap minimal harus 6 karakter.";
    }

    // 2. Validasi Email (Sederhana)
    QString email = ui->emailEdit->text().trimmed();
    if (email.isEmpty()) {
        errors << "- Email tidak boleh kosong.";
    } else if (!email.contains('@') || !email.contains('.')) {
        errors << "- Format Email tidak valid.";
    }

    // 3. Validasi Nomor Telepon
    QString phone = ui->phoneEdit->text().trimmed();
    if (phone.isEmpty()) {
        errors << "- Nomor Telepon tidak boleh kosong.";
    }

    // Tampilkan semua error jika ada
    if (!errors.isEmpty()) {
        QMessageBox::warning(this, "Peringatan Validasi", 
                             "Mohon perbaiki kesalahan berikut:\n\n" + errors.join("\n"));
        return;
    }

    // Jika lolos validasi, kumpulkan parameter yang berubah
    QVariantMap updateParams;
    
    if (fullname != info.nama_lengkap) {
        updateParams["nama_lengkap"] = fullname;
    }
    if (phone != info.nomor_telp) {
        updateParams["nomor_telp"] = phone;
    }
    if (email != info.email) {
        updateParams["email"] = email;
    }
    
    // Tambahkan pengecekan Role jika berubah
    int currentRoleId = ui->roleBox->currentRoleId(); // Asumsi method ini ada di custom widget Anda
    if (currentRoleId != info.role_id) {
        updateParams["role_id"] = currentRoleId;
    }

    // Jika tidak ada perubahan sama sekali, langsung tutup (opsional)
    if (updateParams.isEmpty()) {
        accept();
        return;
    }

    // Eksekusi CRUD
    AdminManager aa;
    if (!aa.update(info.id, updateParams)) {
        QMessageBox::critical(this, "Update Gagal", aa.errorString());
        return;
    }

    accept();
}

void EditUserDialog::userNotFound() {
  QMessageBox::critical(this, "Kesalahan", "Username tidak dikenali");
  reject();
}


void EditUserDialog::setEditRoleDisabled(bool disable) {
  ui->roleBox->setDisabled(disable);
}