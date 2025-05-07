#include "createaccountdialog.h"
#include "files/ui_createaccountdialog.h"
#include "usermanager.h"
#include <QMessageBox>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlError>


CreateAccountDialog::CreateAccountDialog(QWidget* parent) :
ui(new Ui::CreateAccountDialog), QDialog(parent) {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
}

CreateAccountDialog::~CreateAccountDialog() {
    delete ui;
}

void CreateAccountDialog::on_saveButton_clicked() {
    auto aName = ui->accountEdit->text().trimmed();
    auto aDisp = ui->displayEdit->text().trimmed();
    auto pass = ui->passwordEdit->text().trimmed();
    auto repass = ui->repasswordEdit->text().trimmed();
    if(aName.count() < 5) {
        QMessageBox::information(this, tr("Info"), tr("Nama Akun tidak diterima, masukan minimal 5 huruf"));
        return;
    }
    if(pass.count() < 5) {
        QMessageBox::information(this, tr("Info"), tr("Password tidak diterima, masukan minimal 5 huruf"));
        return;
    }
    if(pass != repass) {
        QMessageBox::information(this, tr("Info"), tr("Password tidak cocok, pastikan anda mengetiknya dengan benar"));
        ui->repasswordEdit->setFocus(Qt::OtherFocusReason);
        return;
    }
    if(aDisp.isEmpty()) {
        QMessageBox box(QMessageBox::Question,
                    tr("Konfirmasi"), 
                    tr("Anda tidak memberikan Nama Display, Nama Akun akan dipilih secara Default ?"),
                    QMessageBox::Yes | QMessageBox::No);
        box.setButtonText(QMessageBox::Yes, "Ya");
        box.setButtonText(QMessageBox::No, "Tidak");
        ui->displayEdit->setText(aName);            
        if(box.exec(); != QMessageBox::Yes) {
            ui->displayEdit->selectAll();
            return;
        }
    }
    QSqlTableModel model;
    model.setTable("users");
    
    auto rec = model.record();
    accept();
}