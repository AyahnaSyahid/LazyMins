#include "revokepassworddialog.h"
#include "ui_revokepassworddialog.h"
#include "src/utils/sessionmanager.h"
#include "src/utils/authmanager.h"
#include <QMessageBox>

RevokePasswordDialog::RevokePasswordDialog(QWidget *p) :
ui(new Ui::RevokePasswordDialog), QDialog(p)
{
  ui->setupUi(this);
};

RevokePasswordDialog::~RevokePasswordDialog() { delete ui; }

void RevokePasswordDialog::on_konfirmasiButton_clicked() {
  QString pass = ui->lineEdit->text();
  if(pass.isEmpty()) {
    QMessageBox::information(this, "Ketik password", "Anda belum mengetikkan password anda");
    return ;
  }
  
  auto &sm = SessionManager::instance();
  if (!sm.currentUserPasswordMatch(pass)) {
    QMessageBox::warning(this, "Peringatan", "Password Salah !!");
    return;
  }
  accept();
};