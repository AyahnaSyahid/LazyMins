#include "editcustomerdialog.h"
#include "../ui/ui_editcustomerdialog.h"
#include "customeritem.h"
#include "helper.h"
#include "notifier.h"
#include <QSqlTableModel>
#include <QTableView>
#include <QHeaderView>
#include <QModelIndex>

EditCustomerDialog::EditCustomerDialog(QWidget* parent)
    : ui(new Ui::EditCustomerDialog), QDialog(parent)
{
    ui->setupUi(this);
    comboModel = new QSqlTableModel;
    comboView = new QTableView;
    comboModel->setTable("customers");
    comboModel->select();
    ui->comboBox->setView(comboView);
    ui->comboBox->setModel(comboModel);
    ui->comboBox->setModelColumn(1);
    comboView->hideColumn(0);
    comboView->hideColumn(5);
    comboView->hideColumn(6);
    comboView->horizontalHeader()->hide();
    comboView->resizeColumnsToContents();
    comboView->setMinimumWidth(comboView->horizontalHeader()->length());
    comboView->verticalHeader()->setMinimumSectionSize(15);
    comboView->verticalHeader()->setDefaultSectionSize(16);
    comboView->verticalHeader()->hide();
    comboView->setSelectionBehavior(comboView->SelectRows);
    // comboView->setMinimumHeight(50);
    ui->comboBox->setCurrentIndex(-1);
}

EditCustomerDialog::~EditCustomerDialog() {
    delete ui;
    delete comboView;
    delete comboModel;
}

void EditCustomerDialog::on_pushButton_clicked() {
    if(ui->lineEdit->text().trimmed().isEmpty()  ||
       ui->plainTextEdit->toPlainText().trimmed().isEmpty()) {
           MessageHelper::information(nullptr, "Kesalahan", "Nama dan Alamat harus di isi");
           return;
    }
    auto mi = comboModel->index(ui->comboBox->currentIndex(), 0);
    if(!mi.isValid()) return;
    CustomerItem ci(mi.data(Qt::EditRole).toLongLong());
    ci.customer_name = ui->lineEdit->text().simplified();
    ci.address = ui->plainTextEdit->toPlainText().simplified();
    ci.phone_number = ui->lineEdit_2->text().trimmed();
    ci.email = ui->lineEdit_4->text().trimmed();
    
    if(ci.save()) {
        MessageHelper::information(nullptr, "Berhasil", "Perubahan data disimpan");
        dbNot->emitChanged("customers");
        accept();
    } else {
        MessageHelper::information(nullptr, "Kesalahan", "Gagal menyimpan perubahan data");
    }
}

void EditCustomerDialog::on_comboBox_currentIndexChanged(int ix) {
    if(ix < 0) {
        ui->lineEdit->clear();
        ui->plainTextEdit->clear();
        ui->lineEdit_2->clear();
        // ui->lineEdit_3->clear();
        ui->lineEdit_4->clear();
    } else {
        CustomerItem ci(comboModel->index(ix, 0).data(Qt::EditRole).toLongLong());
        ui->lineEdit->setText(ci.customer_name);
        ui->plainTextEdit->setPlainText(ci.address);
        ui->lineEdit_2->setText(ci.phone_number);
        // ui->lineEdit_3->setText(comboModel->index(ix, 4).data().toString());
        ui->lineEdit_4->setText(ci.email);
    }
}

int EditCustomerDialog::currentCustomer(){
    if(ui->comboBox->currentIndex() < 0) {
        return 0;
    }
    return comboModel->index(ui->comboBox->currentIndex(), 0).data(Qt::EditRole).toInt();
}

bool EditCustomerDialog::setCurrentCustomer(int cid) {
    auto mil = comboModel->match(comboModel->index(0, 0), Qt::EditRole, cid);
    if(!mil.count()) return false;
    ui->comboBox->setCurrentIndex(mil[0].row());
    ui->comboBox->setDisabled(true);
    return true;
}