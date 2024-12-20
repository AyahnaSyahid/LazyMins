 #include "paymentdialog.h"
#include "ui_paymentdialog.h"
#include "helper.h"
#include "orderitem.h"
#include "invoiceitem.h"
#include "invoiceitem.h"
#include "invoicepaymentsitem.h"
#include "notifier.h"

#include <QAbstractItemModel>
#include <QStandardItemModel>
#include <QSqlRecord>
#include <QLocale>
#include <QSqlQuery>
#include <QSqlError>
#include <QtMath>
#include <QDateTime>
#include <QtDebug>

qreal totalHarga(QAbstractItemModel* mod)
{
    qint64 tHarga = 0;
    for(int i=0; i < mod->rowCount(); ++i) {
        QVariant val = mod->index(i, 0).data();
        if(val.isValid()) {
            qDebug() << i << val;
            OrderItem oi(val.toLongLong());
            if(oi.order_id > 0)
                tHarga += oi.total_price;
        }
    }
    return qCeil(tHarga / 100.0) * 100;
}

PaymentDialog::PaymentDialog(QAbstractItemModel* md, QWidget* parent)
: ui(new Ui::PaymentDialog), transModel(md), QDialog(parent) {
    ui->setupUi(this);
    ui->lineEdit->setText("");
    tHarga = totalHarga(transModel);
    ui->label_7->setText(locale().toString(tHarga, 'g', 9));
    ui->doubleSpinBox3->setFocus();
    updateKembalian();
}

PaymentDialog::~PaymentDialog() {
    delete ui;
}

void PaymentDialog::updateKembalian() {
    bool cukup;
    if(ui->doubleSpinBox3->value() < tHarga - ui->doubleSpinBox2->value()) {
        cukup = false;
    }
    qreal kembalian = ui->doubleSpinBox3->value() - 
                      (tHarga - ui->doubleSpinBox2->value());
    ui->label_9->setText(locale().toString(kembalian, 'g', 16));
    ui->label_8->setText(locale().toString(tHarga - ui->doubleSpinBox2->value(), 'g', 16));
    ui->pushButton->setEnabled(kembalian >= 0); // bayar
    ui->pushButton2->setEnabled(kembalian < 0); // simpan
}

void PaymentDialog::on_pushButton_clicked()
{
    InvoiceItem iit;
    QList<qint64> savedOrders;
    bool ok, error = false;
    QVariant oid;
    iit.user_id = LoginNotifier::currentUser().user_id;
    iit.customer_id = _cusid;
    iit.discount_amount = ui->doubleSpinBox2->value();
    iit.invoice_date = QDateTime::currentDateTime();
    iit.due_date = iit.invoice_date.addDays(3);
    iit.physical_iid = ui->lineEdit->text().trimmed();
    iit.created_date = iit.invoice_date;
    for(int i=0; i<transModel->rowCount(); ++i) {
        oid = transModel->index(i, 0).data(Qt::EditRole);
        if(oid.isValid()) {
            ok = iit.addOrder(oid.toLongLong());
            if(ok) {
                savedOrders << oid.toLongLong();
            } else {
                error = true;
                break;
            }
        }
    }
    iit.save();
    InvoicePaymentsItem ipi;
    // plus discount
    ipi.amount = qMin(ui->doubleSpinBox3->value(), tHarga - ui->doubleSpinBox2->value());
    ipi.method = ui->checkBox->isChecked() ? "transfer" : "cash";
    ipi.payment_date = QDateTime::currentDateTime();
    ipi.created_by = LoginNotifier::currentUser().user_id;
    ipi.created_date = ipi.payment_date;
    ipi.invoice_id = iit.invoice_id;
    ipi.save();
    if(iit.addPayment(&ipi)) {
        for(auto so=savedOrders.cbegin(); so!=savedOrders.cend(); ++so) {
            OrderItem oi(*so);
            oi.invoice_id = iit.invoice_id;
            oi.save();
        }
    }
    iit.updateTotal();
    iit.updatePaid();
    accept();
}

void PaymentDialog::on_pushButton2_clicked()
{ // Bayar
    qDebug() << "Simpan Clicked";
    on_pushButton_clicked();
}

