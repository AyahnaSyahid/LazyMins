#include "addproductdialog.h"
#include "ui_addproductdialog.h"
#include "productitem.h"
#include "helper.h"
#include <QSettings>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlDatabase>

AddProductDialog::AddProductDialog(QWidget* parent) :
    ui(new Ui::AddProductDialog), QDialog(parent)
{
    ui->setupUi(this);
    ui->saveButton->setDefault(true);
}

AddProductDialog::~AddProductDialog() {
    delete ui;
}

void AddProductDialog::on_doneButton_clicked() {
    accept();
}

void AddProductDialog::on_resetButton_clicked() {
    ui->lineEdit->clear();
    ui->lineEdit_2->clear();
    ui->lineEdit_3->clear();
    ui->plainTextEdit->clear();
    ui->spinBox->setValue(0);
    ui->spinBox_2->setValue(0);
}

void AddProductDialog::on_saveButton_clicked() {
    ProductItem p;
    p.code = ui->lineEdit->text().trimmed();
    p.name = ui->lineEdit_2->text().trimmed();
    p.description = ui->plainTextEdit->toPlainText().trimmed();
    p.qr_bar = ui->lineEdit_3->text().trimmed();
    p.use_area = ui->checkBox->isChecked() ? 1 : 0;
    p.cost = ui->spinBox->value();
    p.price = ui->spinBox_2->value();
    
    if(p.code.isEmpty()) {
      ToolTipHelper::showTip(mapToGlobal(ui->lineEdit->geometry().topLeft()),
                       "Kode Barang tidak boleh kosong");
      ui->lineEdit->setFocus();
      return;
    }
    if (p.name.isEmpty()) {
      MessageHelper::information(nullptr, "Kesalahan", "Nama tidak boleh kosong");
      ui->lineEdit_2->setFocus();
      return;
    }
    if (p.description.isEmpty()) {
      auto quest = MessageHelper::question(nullptr, "Konfirmasi", "Deskripsi produk belum diisi\n\nabaikan ?");
      if (quest != QMessageBox::Yes) {
        ui->plainTextEdit->setFocus();
        return;
      }
    }
    if (p.cost >= p.price) {
      MessageHelper::information(nullptr, "Kesalahan", "Harga beli tidak lebih kecil dari harga Jual\n\nIni tidak normal");
      return;
    }
    if (p.price < 1) {
      MessageHelper::information(nullptr, "Kesalahan", "Anda akan menjual barang tak bernilai");
      ui->spinBox_2->setFocus();
      return;
    }
    p.save();  
    if(p.product_id < 1) {
      MessageHelper::information(nullptr, "Gagal menambahkan Produk", "Pastikan anda tidak memasukan nama yang sama dengan produk yang telah tersimpan dalam database");
      return;
    }
    emit productAdded(p.product_id);
    auto quest = MessageHelper::question(nullptr, "Tambahkan lainnya", "Pilih Ya, juka anda ingin menambahkan produk baru lainnya");
    if (quest == QMessageBox::Yes) {
        ui->resetButton->click();
        return;
    }
   accept();
}