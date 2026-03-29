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
    // pastikan hanya user dengan role super_admin yang bisa membuka dialog ini
    // hasValidUserOrClose(this);
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

bool UserDialog::validateFields(const QVariantMap &changes)
{
    auto pass1 = ui->passwordEdit1->text();
    auto pass2 = ui->passwordEdit2->text();
    auto markFieldError = [this](QList<QLineEdit *> fields)
    {
        for (auto field : fields)
        {
            field->setStyleSheet("background-color: #ffcccc;");
        }
        fields.first()->setFocus();
        QTimer::singleShot(2000, [fields]()
                           {
            for (auto field : fields)
            {
                field->setStyleSheet("");
            } });
    };

    QList<QPair<QString, QString>> requiredFields = {
        {"nama_lengkap", "Nama lengkap harus diisi"},
        {"nomor_telp", "Nomor telepon harus diisi"},
        {"email", "Email harus diisi"},
        {"username", "Username harus diisi"},
    };
    // pengecekan harus sesuai dengan urutan field di form agar penandaan errornya benar
    for (const auto &[fieldKey, errorMsg] : requiredFields)
    { 
        auto err = false;
        if (changes.value(fieldKey).toString().isEmpty())
        {
          if (fieldKey == "nama_lengkap") {
            QMessageBox::warning(this, "Error", errorMsg);
            markFieldError({ui->fullnameEdit});
            err = true;
          }
          else if (fieldKey == "nomor_telp"){
            QMessageBox::warning(this, "Error", errorMsg);
            markFieldError({ui->phoneEdit});
            err = true;
          }
          else if (fieldKey == "email") {
            QMessageBox::warning(this, "Error", errorMsg);
            markFieldError({ui->emailEdit});
            err = true;
          }
          else if (fieldKey == "username"){
            QMessageBox::warning(this, "Error", errorMsg);
            markFieldError({ui->usernameEdit});
            err = true;
          }
        }
        if(err) return false;
    }
    if (pass1 != pass2)
    {
        QMessageBox::warning(this, "Error", "Password tidak cocok");
        markFieldError({ui->passwordEdit1, ui->passwordEdit2});
        return false;
    }
    if (pass1.isEmpty() || pass1.length() < 6)
    {
        QMessageBox::warning(this, "Error", "Password harus minimal 6 karakter");
        markFieldError({ui->passwordEdit1, ui->passwordEdit2});
        return false;
    }
    if (isCreateMode() && m_adminManager.exists(changes.value("username").toString()))
    {
        QMessageBox::warning(this, "Error", "Username sudah digunakan");
        markFieldError({ui->usernameEdit});
        return false;
    }
    // username harus lebih dari 6 karakter dalam semua mode
    if (changes.value("username").toString().length() < 6)
    {
        QMessageBox::warning(this, "Error", "Username harus minimal 6 karakter");
        markFieldError({ui->usernameEdit});
        return false;
    }
    return true;
}

bool UserDialog::onSave(const QVariantMap &changes)
{
    if (isCreateMode())
    {
        auto opt = m_adminManager.create(changes);
        if (opt) {
          qDebug() << "Create admin berhasil";
        } else {
          qDebug() << "Create admin gagal";
        }
        return opt.has_value();
    }
    else if (isModifyMode())
    {
        return m_adminManager.update(originalRecord().value("id").toInt(), changes);
    }
    return false;
}

void UserDialog::on_simpanButton_clicked()
{
  auto cl = collect();
  for(auto const& [field, val] : cl.asKeyValueRange()) {
    qDebug().noquote() << field << ':' << val.toString();
  }
  
  auto valid = validateFields(cl);
  if (valid)
  {
    AdminManager am;
    auto opt_adm = am.create(cl);
    if(opt_adm.has_value()) {
      QMessageBox::information(this, "Berhasil", "Admin baru berhasil ditambahkan");
      QDialog::accept();
      return;
    } else {
      QMessageBox::information(this, "Gagal", "Admin baru gagal ditambahkan, Error:\n" + am.errorString());
    }
  } else {
    qDebug() << "Invalid fields";
  }
  return;
}