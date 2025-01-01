#include "editrepaymentdialog.h"
#include "ui_editrepaymentdialog.h"
#include "helper.h"
#include "invoicepaymentsitem.h"
#include "notifier.h"
#include <QDateTime>
#include <QPushButton>
#include <QtDebug>


EditRepaymentDialog::EditRepaymentDialog(qint64 payid, QWidget* parent)
  : ui(new Ui::EditRepaymentDialog), QDialog(parent)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Ubah");
    ui->buttonBox->button(QDialogButtonBox::Ok)->setAutoDefault(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setDefault(false);
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Batal");
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setAutoDefault(false);
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setDefault(false);
    setFixedSize(size());
    setProperty("currentPayId", payid);
    InvoicePaymentsItem ipi(payid);
    if(ipi.method == "cash") {
        ui->labelMetode->setText("Cash");
        ui->comboBox->setCurrentIndex(0);
    } else {
        ui->labelMetode->setText("Transfer");
        ui->comboBox->setCurrentIndex(1);
    }
    ui->labelTanggal->setText(locale().toString(ipi.payment_date, "dddd, d MMM yyyy"));
    ui->labelTerbayar->setText(locale().toCurrencyString(ipi.amount, QString("Rp. ")));
    ui->spinBox->setValue(ipi.amount);
    ui->dateEdit->setDate(ipi.payment_date.date());
    ui->plainTextEdit->setPlainText(ipi.note);
}

EditRepaymentDialog::~EditRepaymentDialog()
{
    delete ui;
}

void EditRepaymentDialog::on_buttonBox_accepted()
{
    InvoicePaymentsItem ipi(property("currentPayId").toLongLong());
    if(ipi.payment_date.date() != ui->dateEdit->date()) {
        ipi.payment_date = QDateTime(ui->dateEdit->date());
    }
    ipi.note = ui->plainTextEdit->toPlainText();
    ipi.amount = ui->spinBox->value();
    ipi.method = ui->comboBox->currentIndex() == 0 ? "cash" : "transfer";
    if (!ipi.save()) {
        MessageHelper::information(this, "Gagal", "Tidak dapat menyimpan perubahan");
        return;
    }
    dbNot->emitChanged("invoice_payments");
    accept();
}