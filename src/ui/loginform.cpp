#include "loginform.h"
#include "usermanager.h"
#include "files/ui_loginform.h"

#include <QMessageBox>

void warning(const char* p, QWidget *w) {
    QMessageBox::warning(nullptr, w->tr("Peringatan"), w->tr(p));
}

LoginForm::LoginForm(UserManager *ll, QWidget *parent)
: uman(ll), ui(new Ui::LoginForm), passFailCount(0), QDialog(parent) {
    ui->setupUi(this);
}

LoginForm::~LoginForm() {
    delete ui;
}

void LoginForm::setAccountName(const QString& n) {
    ui->lineEdit->setText(n);
}

//versi grok
void LoginForm::on_pushButton_clicked() {
    auto username = ui->lineEdit->text().trimmed();
    auto password = ui->lineEdit_2->text().trimmed();

    if (username.isEmpty()) {
        ui->lineEdit->setFocus(Qt::MouseFocusReason);
        warning("Nama Akun harus diisi!", this);
        return;
    }

    if (username.length() < 4) {
        ui->lineEdit->setFocus(Qt::MouseFocusReason);
        warning("Nama Akun terlalu pendek, minimal 4 huruf", this);
        return;
    }

    if (!uman->nameExists(username)) {
        ui->lineEdit->setFocus(Qt::MouseFocusReason);
        warning("Nama Akun tidak terdaftar", this);
        return;
    }

    if (password.isEmpty()) {
        ui->lineEdit_2->setFocus(Qt::MouseFocusReason);
        warning("Anda belum meng-input Kata Sandi!", this);
        return;
    }

    if (password.length() < 4) {
        ui->lineEdit_2->setFocus(Qt::MouseFocusReason);
        warning("Kata Sandi terlalu pendek, minimal 4 huruf", this);
        return;
    }

    try {
        if (uman->login(username, password)) {
            passFailCount = 0; // Reset counter on successful login
            accept();
        } else {
            passFailCount++;
            if (passFailCount < 3) {
                warning("Nama Akun atau Kata Sandi tidak cocok.", this);
            } else {
                warning("Anda telah gagal masuk sebanyak 3 kali. Aplikasi akan ditutup.", this);
                reject();
            }
        }
    } catch (const std::exception& e) {
        auto p = QString("Terjadi kesalahan saat login: %1").arg(e.what());
        warning(p.toStdString().c_str(), this);
    }
}
/* 
void LoginForm::on_pushButton_clicked() {
    auto username = ui->lineEdit->text();
    auto password = ui->lineEdit_2->text();
    if(username.isEmpty()) {
        return warning("Nama Akun harus diisi !", this);
    }
    
    if(username.count() < 4) {
        return warning("Nama Akun terlalu pendek, Minimal 4 huruf", this);
    }
    
    if(password.isEmpty()) {
        ui->lineEdit_2->setFocus(Qt::MouseFocusReason);
        return warning("Anda belum meng-input Kata Sandi !", this);
    }
    
    if(password.count() < 4) {
        return warning("Kata Sandi terlalu pendek, Minimal 4 huruf", this);
    }
    
    if( !uman->nameExists(username.trimmed()) ) {
        return warning("Nama Akun tidak terdaftar", this);
    }
    
    if( uman->login(username.trimmed(), password.trimmed()) ) {
      accept();
      return;
    } else {
      if(passFailCount < 3) {
          passFailCount++;
          warning("Nama Akun atau Kata Sandi tidak cocok.", this);
          return;
      } else {
        warning("Anda telah gagal masuk sebanyak 3 kali, Aplikasi akan ditutup", this);
        reject();
      }
    }
} */