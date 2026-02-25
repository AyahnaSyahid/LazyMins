#include "edituserdialog.h"
#include "ui_edituserdialog.h"

EditUserDialog::EditUserDialog(const QString& name, QWidget *p):
ui(new Ui::EditUserDialog), QDialog(p)
{
  ui->setupUi(this);
  setProperty("currentUsername", name);
}

EditUserDialog::~EditUserDialog() { delete ui; }

void EditUserDialog::on_ubahButton_clicked() {}
void EditUserDialog::on_simpanButton_clicked() {}