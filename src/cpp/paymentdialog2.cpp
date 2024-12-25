#include "paymentdialog2.h"
#include "ui_paymenteditdialog.h"
#include "helper.h"

#include "invoiceitem.h"
#include "notifier.h"

#include <QtDebug>

PaymentDialog2::PaymentDialog2(QWidget* parent)
: ui(new Ui::PaymentEditDialog), QDialog(parent){
    ui->setupUi(this);
}

PaymentDialog2::~PaymentDialog2(){
    delete ui;
}

void PaymentDialog2::setPayment(qint64 pid) {
    InvoiceItem ii(pid);
    if (ii.invoice_id < 1) {
        MessageHelper::warning(this, "Kesalahan", QString("Invoice dengan id:%1 tidak dapat ditemukan").arg(pid));
        reject();
    } else {
        ii.updateTotal();
        ii.updatePaid();
        ui->dateEdit->setDate(ii.invoice_date.date());
        ui->lineEdit->setText(ii.physical_iid);
        ui->labelTotal->setText(locale().toString(ii.total_amount));
        ui->doubleSpinBox->setValue(ii.discount_amount);
        ui->labelNet->setText(locale().toString(ii.total_amount - ii.discount_amount));
        adjustSize();
    }
    inv_id = pid;
}

void PaymentDialog2::on_doubleSpinBox_valueChanged(qreal v) {
    InvoiceItem inv(inv_id);
    ui->labelNet->setText(locale().toString(inv.total_amount - v));
}

void PaymentDialog2::on_pushButton_clicked()
{
    InvoiceItem ii(inv_id);
    auto new_disc = ui->doubleSpinBox->value();
    auto new_phy = ui->lineEdit->text();
    auto new_date = ui->dateEdit->date();
    if(new_disc == ii.discount_amount and new_phy == ii.physical_iid and new_date == ii.invoice_date.date()) {
        // no change
    } else {
        ii.discount_amount = new_disc;
        ii.physical_iid = new_phy;
        ii.invoice_date = QDateTime(new_date);
        if(ii.save()) {
            ii.updateTotal();
            ii.updatePaid();
            dbNot->emitChanged("invoices");
        } else {
            MessageHelper::warning(this, "Kesalahan", "Tidak dapat menyimpan data");
            return;
        }
    }
    accept();
}