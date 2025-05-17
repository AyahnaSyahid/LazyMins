#include "database.h"
#include "usermanager.h"
#include "createinvoicedialog.h"
#include "files/ui_createinvoicedialog.h"
#include "models/createordermodel.h"

#include <QItemSelectionModel>
#include <QDate>
#include <QtDebug>
#include <QStandardItemModel>

CreateInvoiceDialog::CreateInvoiceDialog(QItemSelectionModel* sel, Database* _d, QWidget* parent) :
db(_d), ui(new Ui::CreateInvoiceDialog), orderIds(), om(qobject_cast<CreateOrderModel*>(sel->model())), sm(sel), QDialog(parent) {
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

void CreateInvoiceDialog::on_saveButton_clicked() {
    QString where("order_id IN (%1)");
    QStringList oid_t;
    for(auto nid=orderIds.cbegin(); nid != orderIds.cend(); ++nid) {
        oid_t << QString::number(*nid);
    }
    where = where.arg(oid_t.join(", "));
    
    qDebug() << "WHERE STMT : " << where;
    
    QSqlQuery q("BEGIN;");
    q.prepare("SELECT count(invoice_id) + 1 FROM invoices WHERE invoice_date = ?");
    q.addBindValue(ui->invoiceDate->date().toString("yyyy-MM-dd"));
    q.exec() && q.next();
    
    int invId = q.value(0).toInt();
    
    q.prepare("INSERT INTO invoices (invoice_id, invoice_date, customer_id, user_id, total_amount, notes) VALUES (?, ?, ?, ?, ?, ?);");
    q.addBincValue()
    
}

void CreateInvoiceDialog::on_payButton_clicked() {
    
}