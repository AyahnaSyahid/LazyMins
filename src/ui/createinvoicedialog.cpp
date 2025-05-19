#include "database.h"
#include "usermanager.h"
#include "createinvoicedialog.h"
#include "files/ui_createinvoicedialog.h"
#include "models/createordermodel.h"

#include <QDate>
#include <QMessageBox>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlError>

#include <QtDebug>

CreateInvoiceDialog::CreateInvoiceDialog(int cid, QItemSelectionModel* sel, Database* _d, QWidget* parent) :
db(_d), ui(new Ui::CreateInvoiceDialog), orderIds(), 
custId(cid), savedInvoiceId(-1), om(qobject_cast<CreateOrderModel*>(sel->model())), sm(sel), QDialog(parent) {
    ui->setupUi(this);
    ui->invoiceDate->setDate(QDate::currentDate());

    auto stdModel = new QStandardItemModel(om->rowCount(), om->columnCount(), this);
    stdModel->setObjectName("stdModel");
    
    if(sm->hasSelection()) {
        // setup data by selected rows
        auto rows = sm->selectedRows();
        stdModel->setRowCount(rows.count());
        auto currow = 0;
        for(auto r = rows.cbegin(); r != rows.cend(); ++r, ++currow) {
            for(auto c = 0; c < om->columnCount(); ++c) {
                auto omix = (*r).siblingAtColumn(c);
                auto six = stdModel->index(currow, c);
                stdModel->setData(six, omix.data(Qt::EditRole), Qt::EditRole);
                stdModel->setData(six, omix.data(Qt::TextAlignmentRole), Qt::TextAlignmentRole);
                stdModel->setData(six, omix.data(Qt::DisplayRole), Qt::DisplayRole);
            }
            orderIds << (*r).siblingAtColumn(0).data(Qt::EditRole).toInt();
        }
    } else {
        for(int r =0; r < om->rowCount(); ++r) {
            for(int c=0; c < om->columnCount(); ++c) {
                auto omix = om->index(r, c);
                auto six = stdModel->index(r, c);
                stdModel->setData(six, omix.data(Qt::TextAlignmentRole), Qt::TextAlignmentRole);
                stdModel->setData(six, omix.data(Qt::EditRole), Qt::EditRole);
                stdModel->setData(six, omix.data(Qt::DisplayRole), Qt::DisplayRole);
            }
            orderIds << om->index(r, 0).data(Qt::EditRole).toInt();
        }
    }

    ui->orderItemView->setModel(stdModel);
    ui->orderItemView->hideColumn(0);
    
    for(int c = 0; c < om->columnCount(); ++c) {
        // stdModel->setHeaderData(c, Qt::Horizontal, om->headerData(c, Qt::Horizontal, Qt::DisplayRole), Qt::DisplayRole);
        stdModel->setHeaderData(c, Qt::Horizontal, om->headerData(c, Qt::Horizontal, Qt::DisplayRole), Qt::EditRole);
    }
    calculateTotal();
    connect(stdModel, &QAbstractItemModel::dataChanged, this, &CreateInvoiceDialog::calculateTotal);
    connect(ui->spinDiscount, SIGNAL(valueChanged(int)), SLOT(calculateTotal()));
    ui->lItemCount->setText(QString("Jumlah item : %1").arg(stdModel->rowCount()));
    
    UserManager* uman = db->findChild<UserManager*>("userManager");
    ui->lUserDisplay->setText("Invalid UserID");
    if(uman) {
        const QSqlRecord rc = uman->currentUserRecord();
        if(rc.isEmpty()) {
            ui->lUserDisplay->setText("Invalid UserID");
        } else {
            ui->lUserDisplay->setText(rc.value("display_name").toString());
        }
    }
}

CreateInvoiceDialog::~CreateInvoiceDialog() {
    delete ui;
}

void CreateInvoiceDialog::calculateTotal() {
    if(sm->hasSelection()) {
        qint64 ls = om->sum(sm->selectedRows());
        ui->lSubT->setText(locale().toString(ls));
        ui->lGrandT->setText(locale().toString(ls - ui->spinDiscount->value()));
    } else {
        qint64 ls = om->sum();
        ui->lSubT->setText(locale().toString(ls));
        ui->lGrandT->setText(locale().toString(ls - ui->spinDiscount->value()));
    }
    ui->orderItemView->resizeColumnsToContents();
    ui->orderItemView->resizeRowsToContents();
}

void CreateInvoiceDialog::on_cancelButton_clicked() {
    reject();
}

bool CreateInvoiceDialog::saveInvoice(QSqlError* ref) {
        auto uman = db->findChild<UserManager*>("userManager");
    QString where("order_id IN (%1)");
    QStringList oid_t;
    for(auto nid=orderIds.cbegin(); nid != orderIds.cend(); ++nid) {
        oid_t << QString::number(*nid);
    }
    where = where.arg(oid_t.join(", "));
    
    qDebug() << "WHERE STMT : " << where;
    
    QSqlQuery q("BEGIN;");
    q.prepare("INSERT INTO invoices (daily_enum, invoice_date, customer_id, user_id, total_amount, discount, notes)"
        "VALUES ((SELECT COALESCE(MAX(daily_enum), 0) + 1 FROM invoices WHERE invoice_date = ?), ?, ?, ?, ?, ?, ?);");
    
    q.addBindValue(ui->invoiceDate->date().toString("yyyy-MM-dd"));
    q.addBindValue(ui->invoiceDate->date().toString("yyyy-MM-dd"));
    q.addBindValue(custId);
    q.addBindValue(uman->currentUser());
    auto price = sm->hasSelection() ? om->sum(sm->selectedRows()) : om->sum();
    q.addBindValue(price);
    q.addBindValue(ui->spinDiscount->value());
    q.addBindValue(ui->plainTextEdit->toPlainText().simplified());
    if(!q.exec()) {
        QMessageBox::information(this, "Gagal", QString("Tidak dapat menyimpan invoice\nError:\n%1").arg(q.lastError().text()));
        q.exec("ROLLBACK");
        return false;
    }
    savedInvoiceId = q.lastInsertId().toInt();
    QString upd("UPDATE orders SET invoice_id = last_insert_rowid() WHERE %1");
    q.prepare(upd.arg(where));
    if(!q.exec()) {
        QMessageBox::information(this, "Gagal", QString("Tidak dapat mengupdate orders\nError:\n%1").arg(q.lastError().text()));
        q.exec("ROLLBACK");
        savedInvoiceId = -1;
        return false;
    }
    q.exec("COMMIT");
    db->getTableModel("invoices")->select();
    db->getTableModel("orders")->select();
    om->reload();
    return true;
}

void CreateInvoiceDialog::on_saveButton_clicked() {
    QSqlError err;
    if(saveInvoice(&err)) {
        accept();
    }
}

void CreateInvoiceDialog::on_payButton_clicked() {
    QSqlError err;
    if(!saveInvoice(&err)) {
        return;
    }
    if(savedInvoiceId > -1) {
        emit openPayment(savedInvoiceId);
    }
}