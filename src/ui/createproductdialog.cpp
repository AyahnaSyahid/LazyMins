#include "createproductdialog.h"
#include "files/ui_createproductdialog.h"
#include "database.h"
#include <QSqlTableModel>
#include <QMessageBox>
#include <QSqlRecord>
#include <QSqlError>
#include <QtDebug>

CreateProductDialog::CreateProductDialog(Database* _d, QWidget* parent) :
db(_d), ui(new Ui::CreateProductDialog), QDialog(parent) {
    ui->setupUi(this);
}

CreateProductDialog::~CreateProductDialog() {
    delete ui;
}

void CreateProductDialog::on_pushButton_clicked() {
    QString codeField = ui->codeField->text().simplified();
    if(codeField.isEmpty()) {
        QMessageBox::information(this, tr("Kesalahan Input"), tr("Kode Produk tidak boleh kosong"));
        return;
    }
    if(codeField.count() < 5) {
        QMessageBox::information(this, tr("Kesalahan Input"), tr("Kode Produk terlalu pendek, minimal 5 huruf"));
        return;
    }
    QString descField = ui->descriptionField->toPlainText().simplified();
    if(descField.isEmpty()) {
        QMessageBox::information(this, tr("Kesalahan Input"), tr("Deskripsi Produk tidak boleh kosong"));
        return;
    }
    
    int cost = ui->spinCost->value(),
        price = ui->spinPrice->value();
    
    if(cost > price) {
        QMessageBox::information(this, tr("Kesalahan Input"), tr("Harga produksi dibawah harga jual"));
        return;
    }
    
    QString material = ui->materialField->text().simplified(),
            category = ui->categoryField->text().simplified();
    
    
    if(material.isEmpty() || category.isEmpty()) {
        QMessageBox question(QMessageBox::Question, tr("Konfirmasi"),
            tr(QString("Simpan data produk tanpa memberikan info %1").arg(material.isEmpty() ? tr("Kategori") : tr("Material")).toLatin1()),
            QMessageBox::Yes | QMessageBox::No, this);
        
        question.setButtonText(QMessageBox::Yes, "Ya");
        question.setButtonText(QMessageBox::No, "Tidak");
        if(question.exec() == QMessageBox::No) {
            return ;
        }
    }
    
    auto model = db->getTableModel("products");
    auto rec = model->record();
    rec.setValue("name", codeField);
    rec.setValue("description", descField);
    rec.setValue("cost", cost);
    rec.setValue("base_price", price);
    rec.setValue("use_area", ui->areaBox->currentIndex());
    if(! material.isEmpty())
        rec.setValue("material", material);
    if(! category.isEmpty())
        rec.setValue("category", category);
   
    rec.setGenerated("updated_utc", false);
    rec.setGenerated("created_utc", false);
    
    if(model->insertRecord(-1, rec)) {
        if(model->submitAll()) {
            QMessageBox question(QMessageBox::Question, tr("Masukkan data yang lainnya"),
                tr("Lanjut memasukkan data produk lain ?"),
                QMessageBox::Yes | QMessageBox::No, this);
            question.setButtonText(QMessageBox::Yes, "Ya");
            question.setButtonText(QMessageBox::No, "Tidak");
            if(question.exec() == QMessageBox::No) {
                accept();
                return;
            }
            ui->codeField->clear();
            ui->descriptionField->clear();
            ui->categoryField->clear();
            ui->materialField->clear();
            ui->spinCost->setValue(1000);
            ui->spinPrice->setValue(1000);
            return ;
        }
        
        QMessageBox::information(this, tr("Operasi Gagal"), QString("Error : %1").arg(model->lastError().text()));
        return;
    }
    QMessageBox::information(this, tr("Operasi Gagal"), QString("Error : %1").arg("Tidak dapat menyimpan data kedalam database"));    
}