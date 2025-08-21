#include "passworddialogs.h"
#include "files/ui_passworddialogs.h"
#include "usermanager.h"

ChangePasswordDialog::ChangePasswordDialog(UserManager* _uman, QWidget* parent)
: uman(_uman), ui(new Ui::ChangePasswordDialog()), QDialog(parent) {
  ui->setupUi(this);
}

ChangePasswordDialog::~ChangePasswordDialog() {}

void ChangePasswordDialog::on_saveButton_clicked {
  QString crPass = ui->currentPassword->text(),
          nwPass = ui->nwPassword->text(),
          retype = ui->retypePassword->text();
  if(crPass.isEmpty()) {
    QMessageBox::information(this, "Periksa input", "Password lama belum diisi");
    return ;
  } else if(nwPass.isEmpty()){
    QMessageBox::information(this, "Periksa input", "Kata Sandi baru belum di setel");
    return ;
  } else if(retype.isEmpty()) {
    
  }
}