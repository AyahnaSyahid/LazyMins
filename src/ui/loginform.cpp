#include "loginform.h"
#include "usermanager.h"
#include "files/ui_loginform.h"

#include <QMessageBox>

void warning(const char* p, QWidget *w) {
    QMessageBox::warning(nullptr, w->tr("Peringatan"), w->tr(p));
}

LoginForm::LoginForm(UserManager *ll, QWidget *parent)
: uman(ll), ui(new Ui::LoginForm), QDialog(parent) {
    ui->setupUi(this);
}

LoginForm::~LoginForm() {}

void LoginForm::setAccountName(const QString& n) {
    ui->lineEdit->setText(n);
}

void LoginForm::on_pushButton_clicked() {
    auto username = ui->lineEdit->text();
    auto password = ui->lineEdit_2->text();
    if(username.isEmpty()) {
        return warning("Nama Akun harus diisi !", this);
    }
    
    if(username.count() < 5) {
        return warning("Nama Akun terlalu pendek, Minimal 5 huruf", this);
    }
    
    if(password.isEmpty()) {
        return warning("Anda belum meng-input Kata Sandi !", this);
    }
    
    if(password.count() < 5) {
        return warning("Kata Sandi terlalu pendek, Minimal 6 huruf", this);
    }
    
    if( !uman->nameExists(username.trimmed()) ) {
        return warning("Nama Akun tidak terdaftar", this);
    }
    
    if( ! uman->login(username.trimmed(), password.trimmed()) ) {
        return warning("Nama Akun atau Kata Sandi tidak cocok.", this);
    }
    
    accept();
}