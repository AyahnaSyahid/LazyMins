#include "editproductdialog.h"
#include "../ui/ui_editproductdialog.h"
#include "productitem.h"
#include <QSqlQueryModel>
#include <QModelIndex>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlRecord>
#include <QTableView>
#include <QCompleter>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

#include <QtDebug>

EditProductDialog::EditProductDialog(QWidget* parent)
    : ui(new Ui::EditProductDialog), QDialog(parent)
{
    ui->setupUi(this);
    comboView = new QTableView;
    comboModel = new QSqlQueryModel;
    comboModel->setQuery("SELECT product_id, code, name, qr_bar, use_area, description, cost, price FROM products");
    // comboView->setModel(comboModel);
    ui->comboBox->lineEdit()->setAlignment(Qt::AlignCenter);
    ui->comboBox->setModel(comboModel);
    ui->comboBox->setModelColumn(1);
    ui->comboBox->setView(comboView);
    ui->comboBox->setCurrentIndex(-1);
    ui->comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
    comboView->verticalHeader()->setMinimumSectionSize(15);
    comboView->verticalHeader()->setDefaultSectionSize(18);
    comboView->verticalHeader()->hide();
    comboView->setVerticalScrollMode(QTableView::ScrollPerPixel);
    comboView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    comboView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // comboView->setMinimumWidth(comboView->horizontalHeader()->width() + 16);
    comboView->horizontalHeader()->hide();
    comboView->setSelectionBehavior(QTableView::SelectRows);
    comboView->setSelectionMode(QTableView::SingleSelection);
    comboView->hideColumn(0);
    comboView->resizeColumnsToContents();
    comboView->setSizeAdjustPolicy(QTableView::AdjustToContents);
    // comboView->setMinimumWidth(comboView->horizontalHeader()->length() + comboView->autoScrollMargin());
    comboView->setMinimumWidth(comboView->horizontalHeader()->length());
}

EditProductDialog::~EditProductDialog() {
    delete ui;
    delete comboModel;
    delete comboView;
}

void EditProductDialog::on_comboBox_currentIndexChanged(int r) {
    if(ui->comboBox->currentText().toLower() == comboModel->index(r, 1).data(Qt::DisplayRole).toString().toLower()) {
        auto mi = comboModel->index(r, 1);
        ui->lineEdit->setText(mi.data(Qt::DisplayRole).toString());
        ui->lineEdit_2->setText(mi.siblingAtColumn(2).data(Qt::DisplayRole).toString());
        ui->lineEdit_3->setText(mi.siblingAtColumn(3).data(Qt::DisplayRole).toString());
        ui->checkBox->setChecked(mi.siblingAtColumn(4).data().toBool());
        ui->plainTextEdit->setPlainText(mi.siblingAtColumn(5).data(Qt::DisplayRole).toString());
        ui->spinBox->setValue(mi.siblingAtColumn(6).data(Qt::EditRole).toDouble());
        ui->spinBox_2->setValue(mi.siblingAtColumn(7).data(Qt::EditRole).toDouble());
    } else {
        ui->lineEdit->clear();
        ui->lineEdit_2->clear();
        ui->lineEdit_3->clear();
        ui->plainTextEdit->clear();
        ui->spinBox->setValue(0);
        ui->spinBox_2->setValue(0);
        ui->checkBox->setChecked(false);
    }
}

void EditProductDialog::on_saveButton_clicked() {
    if(ui->comboBox->currentIndex() < 0) {
        QMessageBox::warning(nullptr, "Kesalahan", "Pilih barang terlebih dahulu!");
        ui->comboBox->setFocus();
        return;
    }
    if(ui->lineEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(nullptr, "Kesalahan", "Kode Barang harus diisi");
        return;
    }
    if(ui->lineEdit_2->text().trimmed().isEmpty()) {
        QMessageBox::warning(nullptr, "Kesalahan", "Nama Barang harus diisi");
        return;
    }
    if(ui->spinBox->value() >= ui->spinBox_2->value()) {
        QMessageBox::information(nullptr, "Kesalahan", "Harga jual harusnya lebih kecil dari harga beli");
        return;
    }
    ProductItem pi(comboModel->record(ui->comboBox->currentIndex()).value(0).toLongLong());
    if(pi.product_id < 1) {
        QMessageBox::warning(nullptr, "Kesalahan", "Tidak dapat menemukan Produk");
        return;
    }        
    pi.code = ui->lineEdit->text().trimmed();
    pi.name = ui->lineEdit_2->text().trimmed();
    pi.cost =ui->spinBox->value();
    pi.price = ui->spinBox_2->value();
    pi.use_area = ui->checkBox->isChecked() ? 1 : 0;
    pi.description = ui->plainTextEdit->toPlainText().simplified();
    pi.qr_bar = ui->lineEdit_3->text().trimmed();
    bool ok = pi.save();
    if(ok) {
        QMessageBox::information(nullptr, "Notif", "Update Sukses");
        comboModel->setQuery(comboModel->query().lastQuery());
        ui->resetButton->click();
        emit productEdited();
        if(!ui->comboBox->isEnabled()) {
            accept();
        }
    } else {
        QMessageBox::information(nullptr, "Notif", "Update Gagal");
    }
}

void EditProductDialog::on_resetButton_clicked() {
    ui->comboBox->setCurrentIndex(-1);
    ui->lineEdit->clear();
    ui->lineEdit_2->clear();
    ui->lineEdit_3->clear();
    ui->plainTextEdit->clear();
    ui->spinBox->setValue(0);
    ui->spinBox_2->setValue(0);
    ui->comboBox->setFocus();
}

void EditProductDialog::on_doneButton_clicked() {
    accept();
}

int EditProductDialog::currentProduct() {
    if(ui->comboBox->currentIndex() < 0) {
        return -1;
    }
    return comboModel->index(ui->comboBox->currentIndex(), 0).data(Qt::EditRole).toInt();
}

bool EditProductDialog::setCurrentProduct(int pid) {
    auto il = comboModel->match(comboModel->index(0, 0), Qt::EditRole, pid);
    if(il.count()) {
        ui->comboBox->setCurrentIndex(il[0].row());
        ui->comboBox->setDisabled(true);
        return true;
    }
    return false;
}