#include "invoicesmanagerdialog.h"
#include "files/ui_invoicesmanagerdialog.h"
#include "database.h"
#include "models/invoicesmanagermodel.h"
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QSqlTableModel>

InvoicesManagerDialog::InvoicesManagerDialog(Database *_d, QWidget* parent) :
db(_d), inModel(new InvoicesManagerModel(_d, this)), ui(new Ui::InvoicesManagerDialog), QDialog(parent) {
    ui->setupUi(this);
    inModel->setFilterKeyColumn(-1);
    inModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->invoicesView->setModel(inModel);
    ui->invoicesView->hideColumn(0);
    connect(ui->showLunasCheck, SIGNAL(toggled(bool)), inModel, SLOT(showPaidInvoices(bool)));
    connect(db->getTableModel("invoices"), SIGNAL(modelReset()), inModel, SLOT(select()));
    connect(db->getTableModel("payments"), SIGNAL(modelReset()), inModel, SLOT(select()));
    ui->invoicesView->verticalHeader()->hide();
    ui->invoicesView->resizeColumnsToContents();
    ui->invoicesView->setMinimumWidth(ui->invoicesView->horizontalHeader()->length());
    adjustSize();
}

InvoicesManagerDialog::~InvoicesManagerDialog() {
    delete ui;
}

void InvoicesManagerDialog::on_invoicesView_customContextMenuRequested(const QPoint& p) {
    QModelIndex iap(ui->invoicesView->indexAt(p));
    QMenu cm(this);
    if(iap.siblingAtColumn(6).data(Qt::EditRole).toInt() > 0) {
      // Aksi hanya tersedia untuk invoice yang belum lunas
      QAction* act = cm.addAction("Pembayaran");
      act->connect(act, &QAction::triggered, [this, iap](){
          emit db->paymentRequest(iap.siblingAtColumn(0).data(Qt::EditRole).toInt());
      });
    } else {
      // Aksi untuk invoice yang telah lunas
      
    }
    if(cm.isEmpty()) {
        return ;
    }
    cm.exec(ui->invoicesView->viewport()->mapToGlobal(p));
}

void InvoicesManagerDialog::on_eFilter_textChanged(const QString& f) {
  inModel->setFilterFixedString(f);
}