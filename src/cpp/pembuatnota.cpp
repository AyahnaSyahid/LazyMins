#include "pembuatnota.h"
#include "ui_pembuatnota.h"
#include "addorderdialog.h"
#include "flexibletablemodel.h"
#include "paymentdialog.h"
#include "addcustomerdialog.h"
#include "customeritem.h"
#include "notifier.h"
#include "pvordermodel.h"

#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QSqlQueryModel>
#include <QModelIndex>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QHeaderView>
#include <QSizePolicy>
#include <QSqlRecord>
#include <QCompleter>
#include <QTableView>
#include <QLocale>
#include <QTimer>
#include <QDateTime>
#include <QTime>

#include <QtDebug>

PembuatNota::PembuatNota(QWidget *parent)
: ui(new Ui::PembuatNota), ordersModel(new PV_OrderModel(this)), 
  customersModel(new QSqlQueryModel(this)),
  markedModel(new QStandardItemModel(this)),
  QWidget(parent) {
    ui->setupUi(this);
    auto om = qobject_cast<PV_OrderModel*>(ordersModel);
    auto cm = qobject_cast<QSqlQueryModel*>(customersModel);
    auto mm = qobject_cast<QStandardItemModel*>(markedModel);
    cm->setQuery("SELECT customer_id, customer_name, address FROM customers");
    connect(dbNot, &DatabaseNotifier::tableChanged, this, &PembuatNota::onTableChanged);
    connect(dbNot, &DatabaseNotifier::tableChanged, om, &PV_OrderModel::refreshModel);
    ui->comboBox->blockSignals(true);
    ui->comboBox->setModel(customersModel);
    ui->comboBox->setModelColumn(1);
    ui->comboBox->setCurrentIndex(-1);
    ui->comboBox->blockSignals(false);
    ui->comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
    ui->tableView->setModel(ordersModel);
    mm->setColumnCount(8);
    mm->setRowCount(200);
    mm->setHorizontalHeaderLabels(QStringList() << "ID" << "Tanggal" << "Nama" << "Produk" << "W" << "H" << "Qty" << "Nilai");
    ui->tableView2->setModel(markedModel);
    
    ui->tableView->hideColumn(0);
    ui->tableView2->hideColumn(0);
    auto h1 = ui->tableView->horizontalHeader(),
         h2 = ui->tableView2->horizontalHeader();
    h1->setContextMenuPolicy(Qt::CustomContextMenu);
    h2->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(h1, &QHeaderView::customContextMenuRequested, this, &PembuatNota::tableViewHeaderContextMenu);
    connect(h2, &QHeaderView::customContextMenuRequested, this, &PembuatNota::tableViewHeaderContextMenu);
    auto sm = ui->tableView->selectionModel();
    connect(sm, &QItemSelectionModel::selectionChanged, this, &PembuatNota::savedSelectionChanged);
    auto dt = QDateTime::currentDateTime();
    ui->dateEdit->setDate(dt.date());
    ui->timeEdit->setTime(dt.time());
};

PembuatNota::~PembuatNota() {
    ordersModel->deleteLater();
    markedModel->deleteLater();
    customersModel->deleteLater();
    delete ui;
}

void PembuatNota::on_pushButton3_clicked() {
    if(ui->comboBox->currentText().isNull() || ui->comboBox->currentIndex() < 0) {
        return;
    }
    auto mi = qobject_cast<QSqlQueryModel*>(customersModel)->record(ui->comboBox->currentIndex());
    if(mi.isEmpty()) {
        return ;
    }
    auto od = new AddOrderDialog(this);
    od->setCustomer(mi.value(0).toInt());
    od->adjustSize();
    od->setResetButtonEnabled(false);
    connect(od, &AddOrderDialog::accepted, [](){dbNot->emitChanged("orders");});
    od->open();
}

void PembuatNota::reloadModels() {
    auto mm = qobject_cast<QStandardItemModel*>(markedModel);
    auto om = qobject_cast<PV_OrderModel*>(ordersModel);
    om->resetMark();
    mm->setRowCount(0);
    mm->setRowCount(200);
}

void PembuatNota::on_comboBox_currentIndexChanged(int ix) {
    auto mi = customersModel->index(ix, 0);
    auto om = qobject_cast<PV_OrderModel*>(ordersModel);
    reloadModels();
    om->setCustomer(mi.siblingAtColumn(0).data().toLongLong());
    ui->pushButton3->setEnabled(mi.isValid());
}

void PembuatNota::fix_View() {
    ui->tableView->resizeColumnsToContents();
    ui->tableView2->resizeColumnsToContents();
}

void PembuatNota::savedSelectionChanged() {
    QItemSelectionModel *m = qobject_cast<QItemSelectionModel*>(sender());
    if(m) {
        ui->pushButton->setEnabled(m->hasSelection());
    }
}

void PembuatNota::on_pushButton_clicked() {
    auto sm = ui->tableView->selectionModel();
    if(!sm->hasSelection()) return;
    auto mil = sm->selectedRows();
    auto om = qobject_cast<PV_OrderModel*>(ordersModel);
    auto mm = qobject_cast<QStandardItemModel*>(markedModel);
    QList<qint64> mids;
    qint64 lastRow;
    for(int i=0; i<mm->rowCount(); ++i) {
        if(mm->index(i, 0).data(Qt::EditRole).isNull()) {
            lastRow = i;
            break;
        }
        lastRow = i;
    }
    for(auto mi=mil.begin(); mi != mil.end(); ++mi) {
        for(int c=0; c < mm->columnCount(); ++c) {
            mm->setData(mm->index(lastRow, c), mi->siblingAtColumn(c).data(Qt::EditRole), Qt::EditRole);
            mm->setData(mm->index(lastRow, c), mi->siblingAtColumn(c).data(Qt::DisplayRole), Qt::DisplayRole);
            mm->setData(mm->index(lastRow, c), mi->siblingAtColumn(c).data(Qt::TextAlignmentRole), Qt::TextAlignmentRole);
        }
        mids << mi->siblingAtColumn(0).data(Qt::EditRole).toLongLong();
        lastRow++;
    }
    om->marks(mids);
}

void PembuatNota::on_pushButton4_clicked() {
    auto om = qobject_cast<PV_OrderModel*>(ordersModel);
    auto mm = qobject_cast<QStandardItemModel*>(markedModel);
    auto lastCount = mm->rowCount();
    mm->setRowCount(0);
    mm->setRowCount(lastCount);
    om->resetMark();
}

void PembuatNota::on_pushButton5_clicked() {
    if(!markedModel->index(0, 0).data(Qt::EditRole).isValid()) {
        QMessageBox::information(this, "Tidak ada item untuk di bayar", "Pilih item untuk dibayar dengan mengklik 2 kali pada item pada tabel sebelah kiri");
        return;
    }
    auto pd = new PaymentDialog(markedModel, this);
    connect(pd, &PaymentDialog::accepted, [this](){
        dbNot->emitChanged("invoices");
        dbNot->emitChanged("invoice_payments");
        dbNot->emitChanged("orders");
        reloadModels();
    });
    pd->setCustomer(customersModel->index(ui->comboBox->currentIndex(), 0).data().toLongLong());
    pd->open();
}

void PembuatNota::on_tableView_doubleClicked(const QModelIndex& ix) {
    auto om = qobject_cast<PV_OrderModel*>(ordersModel);
    auto mm = qobject_cast<QStandardItemModel*>(markedModel);
    for(int i=0; i<mm->rowCount(); ++i) {
        if(!mm->data(mm->index(i, 0)).isValid()) {
            for(int j=0; j < mm->columnCount(); ++j) {
                mm->setData(mm->index(i, j), ix.siblingAtColumn(j).data(Qt::EditRole), Qt::EditRole);
                mm->setData(mm->index(i, j), ix.siblingAtColumn(j).data(Qt::DisplayRole), Qt::DisplayRole);
                mm->setData(mm->index(i, j), ix.siblingAtColumn(j).data(Qt::TextAlignmentRole), Qt::TextAlignmentRole);
            }
            break;
        }
    }
    om->mark(ix.siblingAtColumn(0).data(Qt::EditRole).toLongLong());
}

void PembuatNota::on_tableView2_doubleClicked(const QModelIndex& ix) {
    auto mm = qobject_cast<QStandardItemModel*>(markedModel);
    auto om = qobject_cast<PV_OrderModel*>(ordersModel);

    om->unmark(ix.siblingAtColumn(0).data(Qt::EditRole).toLongLong());
    for(int i=0; i<mm->columnCount(); ++i) {
        mm->clearItemData(ix.siblingAtColumn(i));
    }
    bool nextRow = true;
    for(int r=ix.row(); nextRow; ++r) {
        auto nr = ix.siblingAtRow(r+1).siblingAtColumn(0);
        if(nr.data(Qt::EditRole).isNull()) {
            nextRow = false;
            break;
        }
        for(int i=0; i<mm->columnCount(); ++i) {
            mm->setData(ix.siblingAtRow(r).siblingAtColumn(i), nr.siblingAtColumn(i).data(Qt::EditRole), Qt::EditRole);
            mm->clearItemData(nr.siblingAtColumn(i));
        }
    }
}

void PembuatNota::on_pushButton2_clicked() {
    auto *acd = new AddCustomerDialog(this);
    acd->setAttribute(Qt::WA_DeleteOnClose);
    connect(acd, &QDialog::accepted, [](){ dbNot->emitChanged("customers"); });
    acd->open();
}

void PembuatNota::markSale(qint64 sid) {
    auto om = qobject_cast<PV_OrderModel*>(ordersModel);
    om->mark(sid);
}

void PembuatNota::tableViewHeaderContextMenu(const QPoint& p) {
    auto hv = qobject_cast<QHeaderView*>(sender());
    if(!hv) return;
    auto gp = hv->viewport()->mapToGlobal(p);
    auto menu = new QMenu();
    QAction* act;
    for(int i=1; i<hv->count(); ++i) {
        act = menu->addAction(ordersModel->headerData(i, Qt::Horizontal).toString());
        act->setProperty("underliyingColumn", i);
        act->setCheckable(true);
        act->setChecked(!hv->isSectionHidden(i));
    }
    act = menu->exec(gp);
    if(act) {
        hv->setSectionHidden(act->property("underliyingColumn").toInt(), !act->isChecked());
    }
}

bool PembuatNota::setCustomer(qint64 cid) {
    CustomerItem ci(cid);
    if(ci.customer_id < 1) return false;
    ui->comboBox->setCurrentText(ci.customer_name);
    return true;
}

void PembuatNota::onTableChanged(const QString& tn) {
     if(tn == "customers") {
            auto cm = qobject_cast<QSqlQueryModel*>(customersModel);
            cm->setQuery(cm->query().lastQuery());
        }
}

