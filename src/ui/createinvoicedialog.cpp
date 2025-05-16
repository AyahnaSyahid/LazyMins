#include "database.h"
#include "createinvoicedialog.h"
#include "files/ui_createinvoicedialog.h"
#include "models/createordermodel.h"

#include <QItemSelectionModel>
#include <QDate>
#include <QtDebug>
#include <QStandardItemModel>

CreateInvoiceDialog::CreateInvoiceDialog(QItemSelectionModel* sel, Database* _d, QWidget* parent) :
db(_d), ui(new Ui::CreateInvoiceDialog), om(qobject_cast<CreateOrderModel*>(sel->model())), sm(sel), QDialog(parent) {
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
        }
    }
    
    ui->orderItemView->setModel(stdModel);
    
    for(int c = 0; c < om->columnCount(); ++c) {
        // stdModel->setHeaderData(c, Qt::Horizontal, om->headerData(c, Qt::Horizontal, Qt::DisplayRole), Qt::DisplayRole);
        stdModel->setHeaderData(c, Qt::Horizontal, om->headerData(c, Qt::Horizontal, Qt::DisplayRole), Qt::EditRole);
    }
    calculateTotal();
    connect(stdModel, &QAbstractItemModel::dataChanged, this, &CreateInvoiceDialog::calculateTotal);
    connect(ui->spinDiscount, SIGNAL(valueChanged(int)), SLOT(calculateTotal()));
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
    
}