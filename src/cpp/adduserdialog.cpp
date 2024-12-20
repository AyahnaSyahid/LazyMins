#include "adduserdialog.h"
#include "ui_adduserdialog.h"
#include "helper.h"
#include "notifier.h"
#include "usermanager.h"
#include "useritem.h"


#include <QDialogButtonBox>
#include <QPushButton>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QToolTip>
#include <QPoint>

#include <QtDebug>

AddUserDialog::AddUserDialog(QWidget* parent) : 
    userItem(new UserItem), ui(new Ui::AddUserDialog), QDialog(parent)
{
    ui->setupUi(this);
    adjustSize();
    auto button = ui->buttonBox->button(QDialogButtonBox::Save);
    button->setText("Simpan");
}

AddUserDialog::~AddUserDialog()
{
    delete ui;
    delete userItem;
}

void AddUserDialog::on_buttonBox_clicked(QAbstractButton* button)
{
    QLineEdit* userLe = findChild<QLineEdit*>("lineEdit");
    QLineEdit* passLe = findChild<QLineEdit*>("lineEdit2");
    if(userLe->text().isEmpty()) {
        auto p = userLe->mapToGlobal(userLe->rect().topLeft());
        QToolTip::showText(p, "<p style='color:red;font:italic bold medium;white-space:pre;'>Username wajib diisi</p>");
        return;
    }
    if(passLe->text().isEmpty()) {
        auto p = passLe->mapToGlobal(passLe->rect().topLeft());
        QToolTip::showText(p, "<p style='color:red;font:italic bold medium;white-space:pre;'>Password wajib diisi</p>");
        return;
    }
    
    if(ui->buttonBox->standardButton(button) == QDialogButtonBox::Save) {
        auto form = ui->userForm;
        QString nname = findChild<QLineEdit*>("lineEdit")->text(),
                npass = findChild<QLineEdit*>("lineEdit2")->text(),
                nfname = findChild<QLineEdit*>("lineEdit3")->text(),
                nemail = findChild<QLineEdit*>("lineEdit4")->text(),
                nphone = findChild<QLineEdit*>("lineEdit5")->text(),
                naddr = findChild<QPlainTextEdit*>("plainTextEdit")->toPlainText();
        if(userItem->user_id > 0) {
            UserItem fu(nname); // find user with name
            if(fu.user_id > 0) {
                QToolTip::showText(mapToGlobal(rect().center()),
                    "<p style='color:red;font:italic bold medium;white-space:pre;'>Nama ini telah dipakai</p>");
                return;
            }
        }
        userItem->username = nname;
        if(!npass.isEmpty())
            userItem->password = npass;
        userItem->full_name = nfname;
        userItem->email = nemail;
        userItem->phone_number = nphone;
        userItem->home_address = naddr;
        if(userItem->save()) {
            // emit userCreated(userItem->user_id);
            dbNot->emitChanged("users");
            accept();
        }
    }
}