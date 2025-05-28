#include "invoicesmanagerdialog.h"
#include "files/ui_invoicesmanagerdialog.h"
#include "database.h"
#include "models/invoicesmanagermodel.h"
#include <QMenu>
#include <QAction>
#include <QSqlTableModel>

InvoicesManagerDialog::InvoicesManagerDialog(Database *_d, QWidget* parent) :
db(_d), inModel(new InvoicesManagerModel(_d, this)), ui(new Ui::InvoicesManagerDialog), QDialog(parent) {
    ui->setupUi(this);
    ui->invoicesView->setModel(inModel);
    ui->invoicesView->resizeColumnsToContents();
    ui->invoicesView->hideColumn(0);
    connect(ui->showLunasCheck, SIGNAL(toggled(bool)), inModel, SLOT(showPaidInvoices(bool)));
    connect(db->getTableModel("invoices"), SIGNAL(modelReset()), inModel, SLOT(select()));
    connect(db->getTableModel("payments"), SIGNAL(modelReset()), inModel, SLOT(select()));
}

InvoicesManagerDialog::~InvoicesManagerDialog() {
    delete ui;
}

void InvoicesManagerDialog::on_invoicesView_customContextMenuRequested(const QPoint& p) {
    QModelIndex iap(ui->invoicesView->indexAt(p));
    QMenu cm(this);
    if(iap.siblingAtColumn(6).data(Qt::EditRole).toInt() > 0) {
        QAction* act = cm.addAction("Bayar");
        act->connect(act, &QAction::triggered, [this, iap](){
            emit db->paymentRequest(iap.siblingAtColumn(0).data(Qt::EditRole).toInt());
        });
    }
    if(cm.isEmpty()) {
        return ;
    }
    cm.exec(ui->invoicesView->viewport()->mapToGlobal(p));
}