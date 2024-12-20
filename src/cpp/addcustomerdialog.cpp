#include "addcustomerdialog.h"
#include "ui_addcustomerdialog.h"
#include "helper.h"
#include "notifier.h"
#include "customeritem.h"
#include <QMenu>
#include <QSqlQuery>
#include <QSqlError>
#include <QtDebug>
#include <QSqlDatabase>

AddCustomerDialog::AddCustomerDialog(QWidget* parent)
    : ui(new Ui::AddCustomerDialog), QDialog(parent)
{
    ui->setupUi(this);
}

AddCustomerDialog::~AddCustomerDialog() {
    delete ui;
}

void AddCustomerDialog::on_pushButton_clicked() {
    if(ui->lineEdit->text().trimmed().isEmpty()) {
        ToolTipHelper::showTip(mapToGlobal(ui->lineEdit->geometry().bottomLeft()),
                "<i style='color:red; font-weight:bold'>Nama tidak boleh di kosongkan</i>");
        ui->lineEdit->setFocus();
        return;
    }
    if(ui->plainTextEdit->toPlainText().trimmed().isEmpty()) {
        ToolTipHelper::showTip(mapToGlobal(ui->lineEdit->geometry().bottomLeft()),
                "<i style='color:red; font-weight:bold'>Alamat tidak boleh di kosongkan</i>");
        ui->plainTextEdit->setFocus();
        return;
    }
    if(ui->lineEdit_2->text().trimmed().isEmpty() || 
        ui->lineEdit->text().trimmed().isEmpty() ||
        ui->lineEdit_4->text().trimmed().isEmpty()) {
        auto ok = MessageHelper::question(nullptr, "Konfirmasi", "Anda mengosongkan beberapa detail kontak\n\nAbaikan dan lanjutkan?");
        if(ok != QMessageBox::Yes) return;
    }
    
    QString nname = ui->lineEdit->text().simplified(),
            naddr = ui->plainTextEdit->toPlainText().simplified(),
            nphone = ui->lineEdit_2->text().trimmed(),
            nemail = ui->lineEdit_4->text().trimmed();
    
    CustomerItem cus(nname);

    if(cus.customer_id > 0) {
        ToolTipHelper::showTip(mapToGlobal(ui->lineEdit->geometry().bottomLeft()),
                "<i style='color:red; font-weight:bold'></i>");
        return;
    }
    cus.email = nemail;
    cus.phone_number = nphone;
    cus.address = naddr;
    cus.created_by = LoginNotifier::currentUser();
    if(cus.save() && cus.customer_id > 0) {
        // emit customerAdded();
        dbNot->emitChanged("customers");
        accept();
    } else {
        qDebug() << QSqlDatabase::database().lastError().text();
    }
}

// void AddCustomerDialog::accept(){
    // QDialog::accept();
// }