#include "createaccountdialog.h"
#include "files/ui_createaccountdialog.h"
#include "usermanager.h"
#include "database.h"
#include <QMessageBox>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlError>


CreateAccountDialog::CreateAccountDialog(Database* _d, QWidget* parent) :
db(_d), ui(new Ui::CreateAccountDialog), QDialog(parent) {
    ui->setupUi(this);
}

CreateAccountDialog::~CreateAccountDialog() {
    delete ui;
}

void CreateAccountDialog::on_saveButton_clicked() {
    auto aName = ui->accountEdit->text().trimmed();
    auto aDisp = ui->displayEdit->text().trimmed();
    auto pass = ui->passwordEdit->text().trimmed();
    auto repass = ui->repasswordEdit->text().trimmed();
    if(UserManager::nameExists(aName)) {
        QMessageBox::critical(this, tr("Info"), tr("Nama Akun %1 telah terdaftar dalam Database").arg(aName));
        return ;
    }
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
        if(box.exec() != QMessageBox::Yes) {
            ui->displayEdit->selectAll();
            return;
        }
    }
    QString eMsg;
    int inid;
    auto userModel = db->getTableModel("users");
    auto uman = db->findChild<UserManager*>("userManager");
    bool createOk = uman->createUser(aName, pass, aDisp, &eMsg, &inid);
    if(!createOk) {
        QMessageBox::critical(this, tr("Kesalahan"), tr("Pembuatan akun gagal : %1").arg(eMsg));
        return;
    } else {
        QMessageBox::information(this, tr("Berhasil"), tr("Akun dengan LOGIN '%1' berhasil dibuat [ID: %2]").arg(aName).arg(inid));
        userModel->select();
        accept();
    }
}