#include "managercustomerdialog.h"
#include "../ui/ui_managercustomerdialog.h"
#include "helper.h"
#include "editcustomerdialog.h"
#include "customermodel.h"
#include "notifier.h"
#include <QSqlTableModel>
#include <QModelIndex>
#include <QMessageBox>
#include <QItemSelectionModel>
#include <QMenu>
#include <QAction>
#include <QtDebug>

ManagerCustomerDialog::ManagerCustomerDialog(QWidget* parent)
    : ui(new Ui::ManagerCustomerDialog), QDialog(parent)
{
    ui->setupUi(this);
    custModel = new CustomerModel(this);
    ui->tableView->setModel(custModel);
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(5);
    ui->tableView->hideColumn(6);
    ui->tableView->setSelectionBehavior(QTableView::SelectRows);
    ui->tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    auto hh = ui->tableView->horizontalHeader();
    connect(hh, &QAbstractItemView::customContextMenuRequested, this, &ManagerCustomerDialog::tableHeaderContextMenu);
}

ManagerCustomerDialog::~ManagerCustomerDialog() {
    delete ui;
    delete custModel;
}

void ManagerCustomerDialog::on_lineEdit_textChanged(const QString& ss) {
    if(ss.isEmpty()) {
        custModel->setFilter("");
        return ;
    }
    custModel->setFilter(QString("LOWER(customer_name || \" \" || address || \" \" || phone_number || \" \" || email) LIKE '%%1%'").arg(ss));
}

void ManagerCustomerDialog::on_pushButton_clicked() {
    if(ui->tableView->selectionModel()->selectedRows(0).count() < 1) {
        MessageHelper::information(nullptr, "Kesalahan", "Pilih data Konsumen yang akan dihapus");
        return;
    }
    auto md = ui->tableView->selectionModel()->selectedRows(0);
    
    auto yes = MessageHelper::question(nullptr, "Konfirmasi", QString("Anda akan menghapus %1 data Konsumen secara permanen\nLanjutkan ?..").arg(md.count()));
    if(yes != QMessageBox::Yes) {
        ui->tableView->clearSelection();
        return;
    }
    std::sort(md.begin(), md.end(), [](const QModelIndex& a, const QModelIndex& b) -> bool {
        return a.row() > b.row();
    });
    bool warn = false;
    for(auto s=md.cbegin(); s!= md.cend(); ++s) {
        custModel->removeRow(s->row());
        if(custModel->submitAll()) {
            custModel->select();
            dbNot->tableChanged("customers");
        } else {
            warn = true;
        }
    }
    if(warn)
        MessageHelper::warning(nullptr, "Tidak diizinkan", "Konsumen mempunyai referensi pembelian");
}

void ManagerCustomerDialog::on_tableView_doubleClicked(const QModelIndex& mi) {
    qDebug() << "Double Clicked";
    EditCustomerDialog* ecd = new EditCustomerDialog(this);
    ecd->setAttribute(Qt::WA_DeleteOnClose);
    if(ecd->setCurrentCustomer(mi.siblingAtColumn(0).data(Qt::EditRole).toInt())) {
        connect(ecd, &QDialog::accepted, custModel, &QSqlTableModel::select);
        ecd->open();
    } else {
        ecd->deleteLater();
    }
}

void ManagerCustomerDialog::tableHeaderContextMenu(const QPoint& pos) {
    QMenu menu(this);
    auto mh = menu.addMenu("Tampilkan");
    auto header = ui->tableView->horizontalHeader();
    auto model = qobject_cast<QSqlTableModel*>(ui->tableView->model());
    for(int i=0; i<header->count(); ++i) {
        auto act = mh->addAction(model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
        act->setCheckable(true);
        act->setProperty("columnIndex", i);
        act->setChecked(!header->isSectionHidden(i));
        connect(act, &QAction::toggled, this, &ManagerCustomerDialog::contextMenuHandler);
    }
    menu.exec(header->mapToGlobal(pos));
}

void ManagerCustomerDialog::contextMenuHandler(bool t) {
    auto *act = qobject_cast<QAction*>(sender());
    auto ci = act->property("columnIndex");
    if(t) {
        ui->tableView->showColumn(act->property("columnIndex").toInt());
    } else {
        ui->tableView->hideColumn(act->property("columnIndex").toInt());
    }
}

void ManagerCustomerDialog::refreshModel()
{
    auto sq = qobject_cast<QSqlTableModel*>(ui->tableView->model());
    sq->select();
}
