#include "managerorderdialog.h"
#include "../ui/ui_managerorderdialog.h"
#include "editorderdialog.h"
#include "pembuatnota.h"
#include "models.h"
#include "helper.h"
#include <QHeaderView>
#include <QModelIndex>
#include <QCompleter>
#include <QScrollBar>
#include <QSqlError>
#include <QSqlQuery>
#include <QLocale>
#include <QMenu>
#include <QAction>
#include <QtDebug>

ManagerOrderDialog::ManagerOrderDialog(QWidget* parent) 
: ui(new Ui::ManagerOrderDialog), QDialog(parent)
{
    ui->setupUi(this);
    omod = new RelationalSaleModel(this);
    ui->tableView->setModel(omod);
    auto thh = ui->tableView;
    thh->hideColumn(0);
    thh->hideColumn(7);
    thh->hideColumn(10);
    thh->hideColumn(11);
    thh->hideColumn(12);

    connect(ui->lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(applyFilter()));
    connect(ui->lineEdit2, SIGNAL(textChanged(const QString&)), this, SLOT(applyFilter()));
    connect(ui->lineEdit3, SIGNAL(textChanged(const QString&)), this, SLOT(applyFilter()));
    connect(ui->checkBox, SIGNAL(toggled(bool)), this, SLOT(applyFilter()));
    connect(ui->checkBox2, SIGNAL(toggled(bool)), this, SLOT(applyFilter()));

    ui->tableView->resizeColumnsToContents();
    applyFilter();
}

ManagerOrderDialog::~ManagerOrderDialog() {
    delete ui;
}

void ManagerOrderDialog::on_tableView_doubleClicked(const QModelIndex& mi){
    EditOrderDialog* eod = new EditOrderDialog(this);
    eod->setOrder(mi.siblingAtColumn(0).data().toLongLong());
    eod->setAttribute(Qt::WA_DeleteOnClose);
    eod->setProdukSelectorEnabled(false);
    eod->exec();
}

void ManagerOrderDialog::on_tableView_customContextMenuRequested(const QPoint& pp) {
    auto gpoint = ui->tableView->viewport()->mapToGlobal(pp);
    auto milist = ui->tableView->selectionModel()->selectedRows(0);
    auto mod = qobject_cast<RelationalSaleModel*>(omod);
    QMenu context;
    if(mod->isSameCustomer(milist) && mod->hasNoPayment(milist)) {
        auto act = context.addAction("Buat Nota");
        act->setObjectName("actBuatNota");
    }
    auto act = context.addAction("Hapus Order");
    act->setObjectName("actHapusOrder");
    act = context.exec(gpoint);
    if(act) {
        auto name = act->objectName();
        if(name == "actBuatNota") {
            QDialog pnd(this);
            PembuatNota pn(&pnd);
            auto geo = pn.geometry();
            pnd.setLayout(pn.layout());
            pnd.setFixedSize(pn.size());
            auto cid = mod->customerId(milist.at(0)).toLongLong();
            pn.setCustomer(cid);
            for(auto mi : milist) {
                pn.markSale(mi.siblingAtColumn(0).data(Qt::EditRole).toLongLong());
            }
            connect(&pn, SIGNAL(notaBaru()), &pnd, SLOT(accept()));
            if(pnd.exec() == QDialog::Accepted) {
                omod->select();
            }
        } else {
            QVariantList ids;
            for(auto mi : milist) {
                ids << mod->idOfIndex(mi);
            }
            // qDebug() << ids;
            auto qq = MessageHelper::question(this, "Konfirmasi", "Beberapa order akan dihapus\nOperasi ini tidak dapat dipulihkan\nLanjutkan dan hapus order terpilih?");
            if(qq == QMessageBox::Yes) {
                // qDebug() << "Ok Clicked";
                mod->removeOrders(ids);
            }
        }
    } else {
        // qDebug() << "No Act";
    }
}

void ManagerOrderDialog::applyFilter() {
    omod->setFilter(filterString());
    omod->setSort(0, ui->checkBox2->isChecked() ? Qt::DescendingOrder : Qt::AscendingOrder);
    omod->select();
}

QString ManagerOrderDialog::filterString() const {
    QStringList ftrs;
    if(!ui->checkBox->isChecked()) {
        ftrs << "payment_id IS NULL";
    }
    if(!ui->lineEdit->text().isEmpty())
        ftrs << QString("customer_name_2 LIKE '%%1%'").arg(ui->lineEdit->text());
    if(!ui->lineEdit2->text().isEmpty()) {
        ftrs << QString("relTblAl_2.code LIKE '%%1%'").arg(ui->lineEdit2->text());
    }
    if(!ui->lineEdit3->text().isEmpty()) {
        ftrs << QString("sale.name LIKE '%%1%'").arg(ui->lineEdit3->text());
    }
    if(ftrs.count() < 1) return "";
    return ftrs.join(" AND ");
}

void ManagerOrderDialog::setNotaId(int m)
{
    omod->setFilter(QString("payment_id = %1").arg(m));
    omod->select();
    ui->groupBox->setEnabled(false);
}