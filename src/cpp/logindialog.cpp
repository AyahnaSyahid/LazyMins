#include "logindialog.h"
#include "ui_logindialog.h"
#include "useritem.h"
#include "loginnotifier.h"

#include "helper.h"

#include <QPushButton>
#include <QDialogButtonBox>

#include <QtDebug>

LoginDialog::LoginDialog(QWidget* parent) :
    ui(new Ui::LoginDialog), QDialog(parent)
{
    ui->setupUi(this);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::accept()
{
    QString name = ui->usernameLineEdit->text(),
            pass = ui->passwordLineEdit->text();
    if(name.isEmpty() | pass.isEmpty()) {
        if(name.isEmpty()) {
            ToolTipHelper::showTip(mapToGlobal(ui->usernameLineEdit->geometry().topLeft()),
             "<p style='font:italic normal bold;color:red;white-space:pre;'>"
             "Anda harus memasukkan Username</p>");
        } else if(pass.isEmpty()) {
            if(ui->usernameLineEdit->hasFocus()) {
                ui->passwordLineEdit->setFocus(Qt::TabFocusReason);
            } else {
                ui->passwordLineEdit->setFocus(Qt::TabFocusReason);
                ToolTipHelper::showTip(mapToGlobal(ui->passwordLineEdit->geometry().topLeft()),
                 "<p style='font:italic normal bold;color:red;white-space:pre;'>"
                 "Anda belum memasukkan password</p>");
            }
        }
    } else {
        UserItem user(name);
        if(user.user_id < 1) {
            ToolTipHelper::showTip( mapToGlobal(ui->usernameLineEdit->geometry().topLeft()),
             "<p style='font:italic normal bold;color:red;white-space:pre;'>Nama pengguna tidak dikenali</p>");
        } else {
            if(!UserItem::passwordMatch(user, pass)) {
                ToolTipHelper::showTip( mapToGlobal(ui->passwordLabel->geometry().bottomLeft()),
                 "<p style='font:italic normal bold;color:red;white-space:pre;'>Username / Password salah!!</p>");
            } else {
                LoginNotifier::instance()->setLoggedUser(user.user_id);
                QDialog::accept();
            }
        }
    }
}


qint64 LoginDialog::blockingLoginDialog(int ma) {

    int attempt = 0;
    qDebug() << __FILE__ << "@" << __LINE__ ;
    qDebug() << "DEV MODE ON";

    // misal rek di deploy comment 1 baris di handap meh aya login dialog
    logNot->setLoggedUser(1);
    return 1;

    qint64& user_id = GlobalNamespace::logged_user_id;

    while(user_id < 0 & attempt < ma) {
        if(attempt)
            MessageHelper::warning(nullptr, "Akses dicekal", "Anda harus masuk dulu");
        auto *ld = new LoginDialog;
        ld->exec();
        attempt++;
        if(attempt == ma)
            MessageHelper::warning(nullptr, "Akses dicekal", "Demi keamanan aplikasi akan segera ditutup\n"
                                                             "jika ini adalah kesalahan hubungi Administrator");
    }

    return user_id;
}
