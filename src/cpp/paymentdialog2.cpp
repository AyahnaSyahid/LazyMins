#include "paymentdialog2.h"
#include "ui_paymentdialog.h"
#include "helper.h"

#include "invoiceitem.h"

#include <QtDebug>

PaymentDialog2::PaymentDialog2(QWidget* parent)
: ui(new Ui::PaymentDialog), QDialog(parent){
    ui->setupUi(this);
}

PaymentDialog2::~PaymentDialog2(){}
void PaymentDialog2::setPayment(qint64 pid) {
    InvoiceItem ii(pid);
    if (ii.invoice_id < 1) {
        MessageHelper::warning(this, "Kesalahan", QString("Invoice dengan id:%1 tidak dapat ditemukan").arg(pid));
        reject();
    } else {
        ii.updateTotal();
        ii.updatePaid();
        ui->lineEdit->setText(ii.physical_iid);
        ui->label_7->setText(locale().toString(ii.total_amount));
        ui->doubleSpinBox2->setValue(ii.discount_amount);
        ui->label_8->setText(locale().toString(ii.total_amount - ii.discount_amount));
        ui->label_3->hide();
        ui->doubleSpinBox3->hide();
        ui->checkBox->hide();
        ui->label_4->hide();
        ui->label_9->hide();
        ui->pushButton->hide();
        adjustSize();
    }
    inv_id = pid;
}

void PaymentDialog2::on_doubleSpinBox2_valueChanged(qreal v) {
    InvoiceItem inv(inv_id);
    ui->label_8->setText(locale().toString(inv.total_amount - v));
}

void PaymentDialog2::on_pushButton2_clicked()
{}