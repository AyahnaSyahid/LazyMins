#include "editcustomerdialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QVariant>
#include "files/ui_createcustomerdialog.h"

EditCustomerDialog::EditCustomerDialog(int cid, QWidget* parent)
    : customerId(0), CreateCustomerDialog(parent)
{
    QSqlQuery q;
    q.prepare("SELECT name, address, phone FROM customers WHERE customer_id = ?");
    q.addBindValue(cid);
    q.exec() && q.next();
    ui->namaLineEdit->setText(q.value("name").toString());
    ui->alamatLineEdit->setText(q.value("address").toString());
    ui->nomorTelpLineEdit->setText(q.value("phone").toString());
}

EditCustomerDialog::~EditCustomerDialog(){}

void EditCustomerDialog::on_pushButton_clicked() {
    QSqlQuery q;
    q.prepare("UPDATE customers SET (name, address, phone) VALUES (?, ?, ?) WHERE customer_id = ?");
    q.addBindValue(ui->namaLineEdit->text());
    q.addBindValue(ui->alamatLineEdit->text());
    q.addBindValue(ui->nomorTelpLineEdit->text());
    q.addBindValue(customerId);
    if(q.exec()) {
        accept();
        return ;
    }
    QMessageBox::information(this, tr("Kesalahan"), q.lastError().isValid() ? q.lastError().text() : tr("Error tidak diketahui"));
}

