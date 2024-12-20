#include "managernota.h"
#include "../ui/ui_managernota.h"
#include "modelpembayarannota.h"
#include "managerorderdialog.h"
#include "helper.h"
#include "tabelrepayment.h"
#include <QMenu>
#include <QAction>
#include <QSqlQuery>
#include <QSqlError>
#include <QDialog>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QtDebug>

ManagerNota::ManagerNota(QWidget *parent) :
    ui(new Ui::ManagerNota), QDialog(parent)
{
        ui->setupUi(this);
        auto mod = new ModelPembayaranNota(this);
        ui->tableView->setModel(mod);
        connect(ui->lunasToggle, &QCheckBox::toggled, mod, &ModelPembayaranNota::lunasToggle);
        connect(ui->checkBox, &QCheckBox::toggled, mod, &ModelPembayaranNota::orderToggle);
        connect(ui->lineEdit, &QLineEdit::textChanged, mod, &ModelPembayaranNota::setFilterName);
        connect(ui->lineEdit2, &QLineEdit::textChanged, mod, &ModelPembayaranNota::setFilterId);
}

ManagerNota::~ManagerNota() {
    delete ui;
}

void ManagerNota::actEditDiscount()
{
    auto mi = ui->tableView->selectionModel()->currentIndex();
    qint64 notaId = mi.siblingAtColumn(0).data(Qt::EditRole).toLongLong();
    
    QString notaIds = mi.siblingAtColumn(0).data(Qt::DisplayRole).toString();
    double currentDiscount = mi.siblingAtColumn(4).data(Qt::EditRole).toDouble();
    double modDiscount = InputHelper::getDouble(this, 
                                                 "Ubah Diskon", 
                                                 QString("Ubah nilai diskon untuk\nNotaId : %1").arg(notaIds), 
                                                 currentDiscount);
    if(qFuzzyCompare(currentDiscount, modDiscount)) {
        return;
    } else {
        QSqlQuery q;
        q.prepare("UPDATE payment SET disc = ? WHERE id = ?");
        q.addBindValue(modDiscount);
        q.addBindValue(notaId);
        q.exec();
        if(q.lastError().isValid()) {
            MessageHelper::information(this, "Update Gagal", q.lastError().text());
        } else {
            updateModel();
        }
    }
}

void ManagerNota::actEditSale()
{
    auto ix = ui->tableView->selectionModel()->currentIndex();
    if(!ix.isValid()) return;
    int paymentId = ix.siblingAtColumn(0).data(Qt::EditRole).toInt();
    ManagerOrderDialog mg(this);
    mg.setNotaId(paymentId);
    mg.exec();
    updateModel();
}

void ManagerNota::actEditPayment()
{
    auto ix = ui->tableView->selectionModel()->currentIndex();
    if(!ix.isValid()) return;
    int paymentId = ix.siblingAtColumn(0).data(Qt::EditRole).toInt();
    
    QDialog dg(this);
    auto tr = new TabelRepayment(&dg);
    tr->setNota(paymentId);
    dg.setLayout(tr->layout());
    dg.setWindowTitle(tr->windowTitle());
    dg.setFixedSize(tr->size());
    dg.exec();
    updateModel();
}

void ManagerNota::actDeleteNota()
{
    auto ix = ui->tableView->selectionModel()->currentIndex();
    if(!ix.isValid()) return;
    int paymentId = ix.siblingAtColumn(0).data(Qt::EditRole).toInt();
    auto a = MessageHelper::question(this, "Konfirmasi", "Anda akan menghapus data pembayaran nota\nLanjut ?");
    if (a == QMessageBox::Yes)
    {
        QSqlQuery q("BEGIN TRANSACTION");
        q.prepare("UPDATE sale SET payment_id = NULL WHERE payment_id = ?");
        q.addBindValue(paymentId);
        q.exec();
        q.prepare("DELETE FROM repayment WHERE payment_id = ?");
        q.addBindValue(paymentId);
        q.exec();
        q.prepare("DELETE FROM payment WHERE id = ?");
        q.addBindValue(paymentId);
        q.exec();
        q.exec("COMMIT");
        if(q.lastError().isValid()) {
            MessageHelper::warning(this, "Gagal menghapus data", q.lastError().text());
        } else {
            MessageHelper::information(this, "Konfirmasi", "Data nota telah dihapus");
            auto mod = qobject_cast<ModelPembayaranNota*>(ui->tableView->model());
            mod->setQuery(mod->query().lastQuery());
        }
        
    }
}

void ManagerNota::on_tableView_customContextMenuRequested(const QPoint& p)
{
    QMenu tcm;
    auto dm = tcm.addMenu("Edit");
    auto sh = tcm.addMenu("Lihat");
    auto disc = dm->addAction("Diskon");
    auto sale = sh->addAction("Penjualan");
    auto pay = sh->addAction("Pembayaran");
    auto del = dm->addAction("Hapus data");
    
    auto acts = tcm.exec(ui->tableView->viewport()->mapToGlobal(p));
    
    if(acts == disc) {
        actEditDiscount();
    } else if (acts == pay) {
        actEditPayment();
    } else if (acts == sale) {
        actEditSale();
    } else if (acts == del) {
        actDeleteNota();
    }
    
}


void ManagerNota::updateModel()
{
    auto mod = qobject_cast<ModelPembayaranNota*>(ui->tableView->model());
    mod->setQuery(mod->query().lastQuery());
}
