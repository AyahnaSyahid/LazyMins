#include "userdialog.h"

#include <QMessageBox>
#include <QTimer>

#include "fieldmap.h"
#include "src/controllers/users.h"
#include "src/customs/buttonguard.h"
#include "src/utils/sessionmanager.h"
#include "ui_userdialog.h"

namespace
{
  // close using QTimer jika SessionManager tidak memiliki user aktif atau valid
  void hasValidUserOrClose(
      QDialog *w,
      const QString &errorMsg = "User tidak valid. Dialog akan ditutup.")
  {
    AdminManager adminManager;
    auto user = SessionManager::instance().currentUser();
    if (!SessionManager::instance().currentUser())
    {
      QMessageBox::warning(w, "User Tidak Valid", errorMsg);
      QTimer::singleShot(0, w, &QDialog::reject);
      return;
    }

    if (!user ||
        !adminManager.userHasRole(user->value("id").toInt(), "super_admin"))
    {
      QMessageBox::warning(w, "Akses Ditolak", errorMsg);
      QTimer::singleShot(0, w, &QDialog::reject);
      return;
    }
  }
} // namespace

UserDialog::UserDialog(QWidget *parent)
    : ui(new Ui::UserDialog), FormDialog(parent)
{
  ui->setupUi(this);
}

UserDialog::~UserDialog() { delete ui; }

void UserDialog::setupFields()
{
  setFields({{ui->fullnameEdit, "nama_lengkap"},
             {ui->phoneEdit, "nomor_telp"},
             {ui->emailEdit, "email"},
             {ui->usernameEdit, "username"},
             {ui->passwordEdit1, "literal_password"}});
}

void UserDialog::setupBoundFields()
{
  addBoundField("role_id", [this]
                { return ui->roleBox->currentRoleId(); }, [this](const QVariant &v)
                { ui->roleBox->setCurrentRoleId(v.toInt()); }, 2); // default KASIR
}

bool UserDialog::isInputAcceptable() const
{
  QStringList err;
  if (ui->fullnameEdit->text().trimmed().isEmpty())
    err << " - Nama lengkap";
  if (ui->phoneEdit->text().trimmed().isEmpty())
    err << " - Nomor Telepon";
  if (ui->emailEdit->text().trimmed().isEmpty())
    err << " - Email";
  if (ui->usernameEdit->text().trimmed().isEmpty())
    err << " - Username";
  if (ui->passwordEdit1->text().trimmed().isEmpty())
    err << " - Password";

  auto pass1 = ui->passwordEdit1->text();
  auto pass2 = ui->passwordEdit2->text();

  if (pass1 != pass2)
    err << "- Password & Verifikasi Harus sama";

  if (err.size())
  {
    QMessageBox::warning(nullptr, "Input Ditolak",
                         QString("Pastikan kriteria berikut terpenuhi:\n%1")
                             .arg(err.join("\n")));
    return false;
  }
  return true;
}

bool UserDialog::onSave(const QVariantMap &changes)
{
  UserController uc;
  QString err;
  QVariantMap cvar(changes);
  if (isCreateMode())
  {
    if (!uc.createUser(cvar, &err))
    {
      QMessageBox::warning(this, "Tidak dapat menyimpan",
                           "Error: \n" + err);
      return false;
    }
  }
  else
  {
    if (!uc.updateUser(originalRecord().value("id").toInt(), changes, &err))
    {
      QMessageBox::warning(this, "Tidak dapat menyimpan\n",
                           "Error:\n" + err);
      return false;
    };
  }
  return true;
}

void UserDialog::on_simpanButton_clicked()
{
  ButtonGuard guard(ui->simpanButton);
  if (!isInputAcceptable())
    return;
  accept();
}