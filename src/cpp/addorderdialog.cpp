#include "addorderdialog.h"
#include "../ui/ui_addorderdialog.h"
#include "addcustomerdialog.h"
#include "tmpsalemodel.h"
#include "customermodel.h"
#include "produkmodel.h"
#include "orderitem.h"
#include "itemdelegates.h"
#include "helper.h"
#include "paymentdialog.h"
#include "databasenotifier.h"
#include <QSqlTableModel>
// #include <QStandardItemModel>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QHeaderView>
#include <QMessageBox>
#include <QItemSelectionModel>
#include <QTableView>
#include <QCompleter>
#include <QSqlError>
#include <QSqlQuery>
#include <QLocale>
#include <QDateTime>

#include <QtDebug>
AddOrderDialog::AddOrderDialog(QWidget *parent)
    : ui(new Ui::AddOrderDialog), QDialog(parent)
{
    ui->setupUi(this);
    custModel = new CustomerModel;
    // custModel->setTable("customers");
    custModel->select();
    ui->comboBox->setModel(custModel);
    ui->comboBox->setModelColumn(1);
    ui->comboBox->setCurrentIndex(-1);
    custView = new QTableView;
    ui->comboBox->setView(custView);
    ui->comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
    ui->comboBox->completer()->setFilterMode(Qt::MatchContains);
    custView->verticalHeader()->setMinimumSectionSize(15);
    custView->verticalHeader()->setDefaultSectionSize(18);
    custView->verticalHeader()->hide();
    custView->setSizeAdjustPolicy(custView->AdjustToContents);
    custView->hideColumn(0);
    custView->hideColumn(3);
    custView->hideColumn(4);
    custView->hideColumn(5);
    custView->hideColumn(6);
    custView->hideColumn(7);
    custView->setMinimumWidth(custView->horizontalHeader()->length());
    custView->horizontalHeader()->hide();
    custView->horizontalHeader()->setStretchLastSection(true);
    custView->resizeColumnsToContents();
    custView->setSelectionBehavior(custView->SelectRows);

    prodModel = new ProdukModel(this);
    ui->comboBoxProduct->setModel(prodModel);
    ui->comboBoxProduct->setModelColumn(1);
    ui->comboBoxProduct->setCurrentIndex(-1);
    ui->comboBoxProduct->completer()->setCompletionMode(QCompleter::PopupCompletion);
    ui->comboBoxProduct->completer()->setFilterMode(Qt::MatchContains);
    prodView = new QTableView;
    ui->comboBoxProduct->setView(prodView);
    prodView->verticalHeader()->setMinimumSectionSize(15);
    prodView->verticalHeader()->setDefaultSectionSize(18);
    prodView->verticalHeader()->hide();
    prodView->setSizeAdjustPolicy(prodView->AdjustToContents);
    prodView->hideColumn(0);
    prodView->hideColumn(3);
    prodView->hideColumn(4);
    prodView->hideColumn(5);
    prodView->hideColumn(6);
    prodView->hideColumn(7);
    prodView->hideColumn(8);
    prodView->hideColumn(9);
    prodView->setMinimumWidth(prodView->horizontalHeader()->length());
    prodView->horizontalHeader()->hide();
    prodView->resizeColumnsToContents();
    prodView->horizontalHeader()->setStretchLastSection(true);
    prodView->setSelectionBehavior(custView->SelectRows);
    
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    tmpModel = new TmpSaleModel(this);
    
    ui->tableView->setModel(tmpModel);
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(1);
    ui->tableView->hideColumn(9);
    ui->tableView->hideColumn(10);
    ui->tableView->hideColumn(11);
    ui->tableView->hideColumn(12);
    ui->tableView->setColumnWidth(1, 200);
    
    ui->tableView->addAction(ui->actionDeleteSelectedItems);
    
    auto itd = new DelegateHarga;
    ui->tableView->setItemDelegateForColumn(8, itd);
    connect(this, &QObject::destroyed, itd, &QObject::deleteLater);
    itd = new DelegateQty;
    ui->tableView->setItemDelegateForColumn(4, itd);
    ui->tableView->setItemDelegateForColumn(5, itd);
    ui->tableView->setItemDelegateForColumn(6, itd);
    ui->pushButtonBayar->hide();
    connect(this, &QObject::destroyed, itd, &QObject::deleteLater);
}

AddOrderDialog::~AddOrderDialog()
{
    delete ui;
    delete custModel;
    delete custView;
    delete prodModel;
    delete prodView;
    delete tmpModel;
}

void AddOrderDialog::setCustomer(int i) {
    int ix = -1;
    ix = custModel->match(custModel->index(0, 0), Qt::EditRole, i)[0].row();
    ui->comboBox->setCurrentIndex(ix);
}

void AddOrderDialog::revertIndex(const QString& iz)
{}

void AddOrderDialog::on_comboBox_currentIndexChanged(int ix)
{
    if (ix == -1)
    {
        ui->comboBox->setDisabled(false);
        ui->groupBox->setEnabled(false);
        ui->pushButtonCustomer->setEnabled(true);
    }
    else
    {
        ui->comboBox->setDisabled(true);
        ui->groupBox->setEnabled(true);
        ui->pushButtonStore->setDisabled(true);
        ui->pushButtonCustomer->setEnabled(false);
    }
}

void AddOrderDialog::on_comboBox_currentIndexChanged(QString s)
{
    if(s.isEmpty())
        ui->comboBox->setCurrentIndex(-1);
}

void AddOrderDialog::on_comboBoxProduct_currentIndexChanged(int ix)
{
    if (ix == -1)
    {
        ui->spinBoxH->setValue(0);
        ui->doubleSpinBoxQ->setValue(0);
        ui->doubleSpinBoxP->setValue(1);
        ui->doubleSpinBoxT->setValue(1);
        ui->lineEdit->clear();
        ui->pushButtonStore->setDisabled(true);
    } else {
        if(!prodModel->index(ix, 6).data(Qt::EditRole).toBool()) {
            hideAreaInput();
        } else {
            showAreaInput();
        }
        ui->lineEdit->setText(prodModel->index(ix, 2).data().toString());
        ui->spinBoxH->setValue(prodModel->index(ix, 5).data().toDouble());
        ui->doubleSpinBoxQ->setValue(1);
        ui->doubleSpinBoxP->setValue(1);
        ui->doubleSpinBoxT->setValue(1);
        ui->pushButtonStore->setDisabled(false);
    }
}

void AddOrderDialog::on_doubleSpinBoxP_valueChanged(double d)
{
    updateTotal();
}

void AddOrderDialog::on_doubleSpinBoxT_valueChanged(double d)
{
    updateTotal();
}

void AddOrderDialog::on_doubleSpinBoxQ_valueChanged(double d)
{
    updateTotal();
}

void AddOrderDialog::on_spinBoxH_valueChanged(int d)
{
    updateTotal();
}

void AddOrderDialog::on_pushButtonCustomer_clicked()
{
    AddCustomerDialog *acd = new AddCustomerDialog(this);
    connect(dbNot, &DatabaseNotifier::tableChanged, this, &AddOrderDialog::updateModel);
    acd->setAttribute(Qt::WA_DeleteOnClose);
    acd->adjustSize();
    acd->open();
}

void AddOrderDialog::on_pushButtonStore_clicked() {
    if(ui->comboBoxProduct->currentIndex() < 0 || ui->lineEdit->text().trimmed().isEmpty()) {
        MessageHelper::information(this, "Kesalahan", "Produk belum ditentukan atau nama tidak ada");
        ui->lineEdit->setFocus();
        ui->lineEdit->selectAll();
        return;
    }
    tmpModel->insertRow(0);
    auto mi = tmpModel->index(0, 0);
    tmpModel->setData(mi.siblingAtColumn(11), custModel->index(ui->comboBox->currentIndex(), 0).data());
    tmpModel->setData(mi.siblingAtColumn(10), prodModel->index(ui->comboBoxProduct->currentIndex(), 0).data());
    tmpModel->setData(mi.siblingAtColumn(2), ui->lineEdit->text().trimmed());
    tmpModel->setData(mi.siblingAtColumn(3), ui->doubleSpinBoxP->value());
    tmpModel->setData(mi.siblingAtColumn(4), ui->doubleSpinBoxT->value());
    tmpModel->setData(mi.siblingAtColumn(5), ui->doubleSpinBoxQ->value());
    tmpModel->setData(mi.siblingAtColumn(9), prodModel->index(ui->comboBoxProduct->currentIndex(), 4).data());
    tmpModel->setData(mi.siblingAtColumn(6), ui->spinBoxH->value());
    tmpModel->setData(mi.siblingAtColumn(1), ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm"));
    ui->comboBoxProduct->setCurrentIndex(-1);
    ui->tableView->resizeColumnsToContents();
}

void AddOrderDialog::on_pushButtonSave_clicked() {
    if(! tmpModel->rowCount()) {
        MessageHelper::information(this, "Kesalahan", "Tidak ada data untuk dicatat");
        return;
    }
    QList<qint64> saveList;
    for(int row = 0; row < tmpModel->rowCount(); ++row) {
        QModelIndex mi = tmpModel->index(row, 0, QModelIndex());
        OrderItem oi;
        oi.customer_id = mi.siblingAtColumn(11).data(Qt::EditRole).toLongLong();
        oi.product_id = mi.siblingAtColumn(10).data(Qt::EditRole).toLongLong();
        oi.order_name = mi.siblingAtColumn(2).data(Qt::EditRole).toString();
        oi.width_mm = mi.siblingAtColumn(3).data(Qt::EditRole).toDouble() * 1000;
        oi.height_mm = mi.siblingAtColumn(4).data(Qt::EditRole).toDouble() * 1000;
        oi.quantity = mi.siblingAtColumn(5).data(Qt::EditRole).toInt();
        oi.cost_production = mi.siblingAtColumn(9).data(Qt::EditRole).toLongLong();
        oi.selling_price = mi.siblingAtColumn(6).data(Qt::EditRole).toLongLong();
        oi.order_date = mi.siblingAtColumn(1).data(Qt::EditRole).toDateTime();
        if(oi.save()) {
            saveList << row;
            emit orderAdded();
        }
    }
    std::sort(saveList.begin(), saveList.end(), [](int a, int b){ return a > b; });
    if(saveList.count() == tmpModel->rowCount()) {
        on_pushButtonReset_clicked();
    }
    dbNot->emitChanged("orders");
}

void AddOrderDialog::on_pushButtonBayar_clicked()
{
    auto pd = new PaymentDialog(tmpModel, this);
    pd->setAttribute(Qt::WA_DeleteOnClose);
    connect(pd, &PaymentDialog::accepted, [](){
        dbNot->emitChanged("invoice_payments");
        dbNot->emitChanged("invoices");
    });
    pd->open();
};

void AddOrderDialog::on_pushButtonReset_clicked()
{
    tmpModel->clear();
    ui->comboBox->setCurrentIndex(-1);
    ui->comboBoxProduct->setCurrentIndex(-1);
}

void AddOrderDialog::updateTotal()
{
    double pr = 
        ui->doubleSpinBoxP->value() *
        ui->doubleSpinBoxT->value() *
        ui->spinBoxH->value() *
        ui->doubleSpinBoxQ->value();
    ui->label_6->setText(locale().toString(pr, 'g', 16));
    // ui->pushButtonBayar->setEnabled(tmpModel->rowCount() > 0);
    if(pr < 1.0) {
        ui->pushButtonBayar->setEnabled(false);
    } else {
        ui->pushButtonBayar->setEnabled(false);
    }
}

void AddOrderDialog::updateModel()
{
    custModel->select();
    prodModel->select();
    tmpModel->clear();
}

void AddOrderDialog::on_actionDeleteSelectedItems_triggered() {
    auto m = ui->tableView->selectionModel();
    if(!m->hasSelection()) return;
    auto il = m->selectedRows(2);
    auto biger = [](const QModelIndex& a, const QModelIndex& b) -> bool { return a.row() > b.row(); };
    std::sort(il.begin(), il.end(), biger);
    for(auto mi=il.begin(); mi!=il.end(); ++mi) {
        tmpModel->removeRow(mi->row());
    }
}

void AddOrderDialog::setResetButtonEnabled(bool e) {
    ui->pushButtonReset->setEnabled(e);
}

void AddOrderDialog::hideAreaInput() {
    ui->label_10->setDisabled(true);
    ui->label_9->setDisabled(true);
    ui->label_11->setDisabled(true);
    ui->doubleSpinBoxP->setDisabled(true);
    ui->doubleSpinBoxT->setDisabled(true);
}

void AddOrderDialog::showAreaInput() {
    ui->label_10->setDisabled(false);
    ui->label_9->setDisabled(false);
    ui->label_11->setDisabled(false);
    ui->doubleSpinBoxP->setDisabled(false);
    ui->doubleSpinBoxT->setDisabled(false);
}

