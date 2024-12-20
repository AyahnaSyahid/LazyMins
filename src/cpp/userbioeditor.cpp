#include "userbioeditor.h"
#include "ui_userbioeditor.h"
#include "helper.h"

UserBioEditor::UserBioEditor(QWidget* parent) :
    ui(new Ui::UserBioEditor), QDialog(parent)
{
    ui->setupUi(this);
    adjustSize();
}

UserBioEditor::~UserBioEditor()
{
    delete ui;
}

void UserBioEditor::setUser(const UserItem& us)
{
    user = us;
    ui->lineEdit3->setText(user.full_name);
    ui->lineEdit4->setText(user.email);
    ui->lineEdit5->setText(user.phone_number);
    ui->plainTextEdit->setPlainText(user.home_address);
}

void UserBioEditor::reject()
{
    bool edited = user.full_name != ui->lineEdit3->text() ||
                  user.email != ui->lineEdit4->text() ||
                  user.phone_number != ui->lineEdit5->text() ||
                  user.home_address != ui->plainTextEdit->toPlainText();
    if (edited) {
        auto ok = MessageHelper::question(this, "Simpan Perubahan", "Beberapa field mengalami perubahan\nSimpan perubahan?");
        if(ok == QMessageBox::Yes) {
            this->accept();
            return;
        }
    }
    QDialog::reject();
}

void UserBioEditor::accept()
{
    user.full_name = ui->lineEdit3->text();
    user.email = ui->lineEdit4->text();
    user.phone_number = ui->lineEdit5->text();
    user.home_address = ui->plainTextEdit->toPlainText();
    user.save();
    QDialog::accept();
}