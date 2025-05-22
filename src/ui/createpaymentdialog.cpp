#include "createpaymentdialog.h"
#include "files/ui_createpaymentdialog.h"
#include "database.h"
#include "models/createordermodel.h"
#include <QDate>
#include <QLocale>

#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QHBoxLayout>
#include <QTableView>
#include <QMessageBox>

const QString prep(R"--(
WITH OrderPrice AS (
    SELECT 
        order_id, 
        invoice_id, 
        CASE 
            WHEN use_area = 1 THEN CEIL(width * height * quantity * unit_price / 500.0) * 500 
            ELSE CEIL(quantity * unit_price / 500.0) * 500 
        END AS real_price
    FROM orders
), PaymentTotal AS (
    SELECT invoice_id, COALESCE(SUM(payments.amount), 0) AS Paid FROM payments GROUP BY invoice_id
)
SELECT 
    invoices.invoice_id,
    customers.name AS CName,
    invoices.notes,
    invoices_dcode.invoice_code AS invoice_code,
    invoices.invoice_date,
    COUNT(orders.order_id) AS ItemCount,
    SUM(OrderPrice.real_price) AS SubT,
    SUM(orders.discount) AS OTDisc,
    invoices.discount AS InvDisc,
    (SUM(orders.discount) + invoices.discount) AS TDisc,
    (SUM(OrderPrice.real_price) - (SUM(orders.discount) + invoices.discount)) AS GrandT,
    COALESCE(Paid, 0) AS Paid,
    (SUM(OrderPrice.real_price) - (SUM(orders.discount) + invoices.discount)) - COALESCE(Paid, 0) AS GRest
FROM invoices
JOIN invoices_dcode ON invoices.invoice_id = invoices_dcode.invoice_id
LEFT JOIN orders ON invoices.invoice_id = orders.invoice_id
LEFT JOIN OrderPrice ON orders.order_id = OrderPrice.order_id
LEFT JOIN PaymentTotal ON invoices.invoice_id = PaymentTotal.invoice_id
LEFT JOIN customers ON invoices.customer_id = customers.customer_id
WHERE invoices.invoice_id = ?
GROUP BY invoices.invoice_id, invoices_dcode.invoice_code, invoices.invoice_date, invoices.discount, PaymentTotal.invoice_id
)--");


CreatePaymentDialog::CreatePaymentDialog(int inv, Database* _d, QWidget* parent) :
invoiceId(inv), db(_d), paymentHistoryModel(new QSortFilterProxyModel(this)), ui(new Ui::CreatePaymentDialog), QDialog(parent) {
    ui->setupUi(this);
    auto paymentModel = db->getTableModel("payments");
    auto mrec = paymentModel->record();
    paymentHistoryModel->setSourceModel(paymentModel);
    paymentHistoryModel->setFilterKeyColumn(mrec.indexOf("invoice_id"));
    // paymentHistoryModel->setFilterRole(Qt::DisplayRole);
    ui->paymentHistoryView->setModel(paymentHistoryModel);
    ui->paymentHistoryView->hideColumn(0);
    ui->paymentHistoryView->hideColumn(2);
    ui->paymentHistoryView->hideColumn(7);
    ui->paymentHistoryView->hideColumn(8);
    paymentHistoryModel->setHeaderData(1, Qt::Horizontal, "Tanggal", Qt::DisplayRole);
    paymentHistoryModel->setHeaderData(1, Qt::Horizontal, "Tanggal pembayaran", Qt::ToolTipRole);
    paymentHistoryModel->setHeaderData(3, Qt::Horizontal, "Nilai", Qt::DisplayRole);
    paymentHistoryModel->setHeaderData(4, Qt::Horizontal, "Metode", Qt::DisplayRole);
    paymentHistoryModel->setHeaderData(5, Qt::Horizontal, "No.Ref", Qt::DisplayRole);
    paymentHistoryModel->setHeaderData(5, Qt::Horizontal, "Referensi BANK (jika ada)", Qt::ToolTipRole);
    paymentHistoryModel->setHeaderData(6, Qt::Horizontal, "Konfirmasi", Qt::DisplayRole);
    fillUiData();
    connect(paymentModel, &QSqlTableModel::modelReset, this, &CreatePaymentDialog::fillUiData);
}

CreatePaymentDialog::~CreatePaymentDialog() {
    delete ui;
}

void CreatePaymentDialog::inputLogic() {
    int currentGRest = property("currentGRest").toInt();
    int spinPayNow = ui->spinPayNow->value();
    int spinRecv = ui->spinRecv->value();
    if((currentGRest - spinPayNow) < 0) {
        ui->saveButton->setEnabled(false);
    } else {
        ui->saveButton->setEnabled(true);
    }
    if(spinRecv >= spinPayNow) {
        ui->lReturn->setText(locale().toString(spinRecv - spinPayNow));
        ui->lNRest->setText(locale().toString(currentGRest - spinPayNow));
    } else {
        ui->lReturn->setText("Tidak Valid");
        ui->lNRest->setText("Tidak Valid");
        if(ui->saveButton->isEnabled()) {
            ui->saveButton->setEnabled(false);
        }
    }
    if(spinRecv == 0) {
        ui->saveButton->setEnabled(false);
    }
}

void CreatePaymentDialog::on_openOrdersView_clicked() {
    QDialog* dlg = new QDialog(this);
    dlg->setLayout(new QHBoxLayout);
    QTableView* tv = new QTableView(dlg);
    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(this);
    auto omod = db->getTableModel("orders");
    proxy->setSourceModel(omod);
    proxy->setFilterKeyColumn(omod->record().indexOf("invoice_id"));
    proxy->setFilterFixedString(QString::number(invoiceId));
    tv->setModel(proxy);
    dlg->layout()->addWidget(tv);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->open();
}

void CreatePaymentDialog::on_saveButton_clicked() {
    auto pm = db->getTableModel("payments");
    auto rec = pm->record();
    rec.setValue("invoice_id", invoiceId);
    rec.setValue("amount", ui->spinPayNow->value());
    rec.setValue("method", ui->methodBox->currentText());
    rec.setGenerated("payment_date", false);
    rec.setValue("confirmed", ui->methodBox->currentText() == "CASH" ? 1 : 0);
    rec.setGenerated("created_utc", false);
    rec.setGenerated("updated_utc", false);
    
    if(pm->insertRecord(-1, rec)) {
        // insert Ok
        if(!pm->submitAll()) {
            if(pm->lastError().isValid()) {
                QMessageBox::critical(this, "Gagal menyimpan data", QString("Error :%1").arg(pm->lastError().text()));
                pm->revertAll();
                return;
            }
            QMessageBox::critical(this, "Gagal menyimpan data", QString("Error :%1").arg("Tidak diketahui"));
            pm->revertAll();
            return;
        }
        QMessageBox::information(this, "Berhasil disimpan", "Pembayaran berhasil disimpan kedalam database");
        return ;
    }
    QMessageBox::critical(this, "Gagal menyimpan data", QString("Error :%1").arg("Tidak diketahui #2"));
    pm->revertAll();
}

void CreatePaymentDialog::fillUiData() {
    ui->spinPayNow->setValue(0);
    ui->spinRecv->setValue(0);
    QSqlQuery q;
    q.prepare(prep);
    q.addBindValue(invoiceId);
    q.exec() && q.next();
    auto irec = q.record();
    auto loc = locale();
    ui->lClient->setText(irec.value("CName").toString());
    ui->lDate->setText(QDate::fromString(irec.value("invoice_date").toString(), "yyyy-MM-dd").toString("dd/MM/yyyy"));
    ui->lCode->setText(irec.value("invoice_code").toString());
    ui->lItem->setText(loc.toString(irec.value("ItemCount").toInt()));
    ui->lSubTotal->setText(loc.toString(irec.value("SubT").toLongLong()));
    ui->lDiscO->setText(loc.toString(irec.value("OTDisc").toLongLong()));
    ui->lDiscI->setText(loc.toString(irec.value("InvDisc").toLongLong()));
    ui->lSumDisc->setText(loc.toString(irec.value("TDisc").toLongLong()));
    ui->lGrandT->setText(loc.toString(irec.value("GrandT").toLongLong()));
    ui->lRecv->setText(loc.toString(irec.value("Paid").toLongLong()));
    auto gr = irec.value("GRest").toInt();
    
    setProperty("currentGRest", gr);
    ui->lRest->setText(loc.toString(gr));
    ui->lNRest->setText(loc.toString(gr));
    ui->pNotes->setPlainText(irec.value("notes").toString());
    ui->spinPayNow->setMaximum(gr);
    paymentHistoryModel->setFilterFixedString(QString::number(invoiceId));
}