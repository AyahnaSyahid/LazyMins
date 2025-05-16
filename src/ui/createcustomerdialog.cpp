#include "createcustomerdialog.h"
#include "files/ui_createcustomerdialog.h"
#include "database.h"
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlError>
#include <QMessageBox>
#include <QPushButton>

CreateCustomerDialog::CreateCustomerDialog(Database* _d, QWidget* parent)
: db(_d), ui(new Ui::CreateCustomerDialog), QDialog(parent) {
    ui->setupUi(this);
}

CreateCustomerDialog::~CreateCustomerDialog() {
    delete ui;
}

void CreateCustomerDialog::on_pushButton_clicked() {
    hide();
    QString nama = ui->namaLineEdit->text(),
            alm  = ui->alamatLineEdit->text(),
            telp = ui->nomorTelpLineEdit->text();

    if(nama.isEmpty() || alm.isEmpty() || telp.isEmpty()) {
        QMessageBox::information(this, tr("Kesalahan input"), tr("Anda diwajibkan mengisi semua Field"));
        show();
        return ;
    }
    if(nama.trimmed().count() < 3) {
        QMessageBox::information(this, tr("Kesalahan input"), tr("Nama Akun terlalu pendek, Minimal 3 Huruf"));
        show();
        return ;
    }
    if(alm.trimmed().count() < 5) {
        QMessageBox::information(this, tr("Kesalahan input"), tr("Alamat terlalu pendek, Minimal 5 Huruf"));
        show();
        return ;
    }
    if(telp.trimmed().count() < 6) {
        QMessageBox::information(this, tr("Kesalahan input"), tr("Alamat terlalu pendek, Minimal 6 Huruf"));
        show();
        return ;
    }
    
    auto model = db->getTableModel("customers");
    auto rec = model->record();
    
    rec.setValue("name", nama);
    rec.setValue("address", alm);
    rec.setValue("phone", telp);
    rec.setGenerated("created_utc", false);
    rec.setGenerated("updated_utc", false);
    
    if(model->insertRecord(-1, rec)) {
        if(!model->submitAll()) {
            QMessageBox::information(this, "Gagal memasukkan data", QString("Error : %1").arg(model->lastError().text()));
            model->revertAll();
            return;
        }
        accept();
    } else {
        QMessageBox::information(this, "Gagal memasukkan data", QString("Error : %1").arg("Tidak diketahui"));
    }
};

void CreateCustomerDialog::reset_input() {
    ui->namaLineEdit->clear();
    ui->alamatLineEdit->clear();
    ui->nomorTelpLineEdit->clear();
}