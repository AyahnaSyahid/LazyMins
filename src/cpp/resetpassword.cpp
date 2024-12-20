#include "resetpassword.h"
#include "ui_resetpassword.h"
#include "helper.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QToolTip>

ResetPassword::ResetPassword(QWidget* parent)
  : ui(new Ui::ResetPassword), QDialog(parent)
{
  ui->setupUi(this);
  auto button = ui->buttonBox->button(QDialogButtonBox::Save);
  button->setText("Simpan");
  // button->setDefault(false);
  // button->setAutoDefault(false);
  button = ui->buttonBox->button(QDialogButtonBox::Cancel);
  button->setText("Batal");
  // button->setDefault(true);
  // button->setAutoDefault(true);
  adjustSize();
}

ResetPassword::~ResetPassword()
{
    delete ui;
}

void ResetPassword::setUser(const UserItem& u)
{
    user = u;
    ui->label_3->setText(u.username);
}

void ResetPassword::accept()
{
    auto pos = mapToGlobal(ui->lineEdit->geometry().center());
    if(!ui->lineEdit->text().isEmpty()) {
        if(ui->lineEdit->text().count() < 6) {
            ToolTipHelper::showTip(pos, "<i style='color:red;'>Password lemah !!, </br>anda harus memasukan password minimal 6 karakter</i>");
            return;
        }
        user.password = ui->lineEdit->text();
        user.save();
        emit passwordChanged(user.user_id);
    } else {
        ToolTipHelper::showTip(pos,
            "<i style='color:red;'>Anda belum memasukan password, minimal 6 karakter</i>");
        return;
    }
    QDialog::accept();
}

void ResetPassword::reject()
{
    QDialog::reject();
}