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
    COALESCE(SUM(payments.amount), 0) AS Paid,
    (SUM(OrderPrice.real_price) - (SUM(orders.discount) + invoices.discount)) - COALESCE(SUM(payments.amount), 0) AS GRest
FROM invoices
JOIN invoices_dcode ON invoices.invoice_id = invoices_dcode.invoice_id
LEFT JOIN orders ON invoices.invoice_id = orders.invoice_id
LEFT JOIN OrderPrice ON orders.order_id = OrderPrice.order_id
LEFT JOIN payments ON invoices.invoice_id = payments.invoice_id
LEFT JOIN customers ON invoices.customer_id = customers.customer_id
WHERE invoices.invoice_id = ?
GROUP BY invoices.invoice_id, invoices_dcode.invoice_code, invoices.invoice_date, invoices.discount
)--");


CreatePaymentDialog::CreatePaymentDialog(int inv, Database* _d, QWidget* parent) :
invoiceId(inv), db(_d), paymentHistoryModel(new QSortFilterProxyModel(this)), ui(new Ui::CreatePaymentDialog), QDialog(parent) {
    ui->setupUi(this);
    QSqlQuery q;
    q.prepare(prep);
    q.addBindValue(inv);
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
    setProperty("currentGRest", irec.value("GRest"));
    ui->lRest->setText(loc.toString(irec.value("GRest").toLongLong()));
    ui->lNRest->setText(loc.toString(irec.value("GRest").toLongLong()));
    ui->pNotes->setPlainText(irec.value("notes").toString());
    auto paymentModel = db->getTableModel("payments");
    auto mrec = paymentModel->record();
    paymentHistoryModel->setSourceModel(paymentModel);
    paymentHistoryModel->setFilterKeyColumn(mrec.indexOf("invoice_id"));
    // paymentHistoryModel->setFilterRole(Qt::DisplayRole);
    paymentHistoryModel->setFilterFixedString(QString::number(inv));
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
    // paymentHistoryModel->setHeaderData(6, Qt::Horizontal, "Transaksi ini telah terkonfirmasi", Qt::ToolTipRole);
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
}

void CreatePaymentDialog::on_openOrdersView_clicked() {
    QDialog* dlg = new QDialog(this);
    dlg->setLayout(new QHBoxLayout);
    QTableView* tv = new QTableView(dlg);
    CreateOrderModel* model = new CreateOrderModel(this);
}