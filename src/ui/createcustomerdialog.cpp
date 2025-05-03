#include "createcustomerdialog.h"
#include "files/ui_createcustomerdialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

CreateCustomerDialog::CreateCustomerDialog(QWidget* parent)
: ui(new Ui::CreateCustomerDialog), QDialog(parent) {
    ui->setupUi(this);
}

CreateCustomerDialog::~CreateCustomerDialog() {
    delete ui;
}

void CreateCustomerDialog::on_pushButton_clicked() {
    hide();
    QString nama = ui->namaLineEdit->text(),
            alm  = ui->alamatLineEdit->text(),
            telp = ui->nomorTeleponLineEdit->text();

    if(nama.isEmpty() || alm.isEmpty() || telp.isEmpty()) {
        QMessageBox::information(nullptr, tr("Kesalahan input"), tr("Anda diwajibkan mengisi semua Field"));
        return ;
    }
    if(nama.trimmed().count() < 3) {
        QMessageBox::information(nullptr, tr("Kesalahan input"), tr("Nama Akun terlalu pendek, Minimal 3 Huruf"));
        return ;
    }
    if(alamat.trimmed().count() < 5) {
        QMessageBox::information(nullptr, tr("Kesalahan input"), tr("Alamat terlalu pendek, Minimal 5 Huruf"));
        return ;
    }
    if(telp.trimmed().count() < 6) {
        QMessageBox::information(nullptr, tr("Kesalahan input"), tr("Alamat terlalu pendek, Minimal 6 Huruf"));
        return ;
    }
    
    QSqlQuery q("BEGIN TRANSACTION;");
    q.prepare("INSERT INTO customers (name, address, phone) VALUES (?, ?, ?)");
    q.addBindValue(nama);
    q.addBindValue(alm);
    q.addBindValue(telp);
    if(!q.exec()) {
        auto err = q.lastError();
        if(err.isValid()) {
            if(err.text().contains("Unique_ID_Customer", Qt::CaseInsensitive)) {
                QMessageBox::information(nullptr, tr("Duplikasi"), tr("Konsumen dengan nama, alamat dan nomor telepon ini telah tersimpan dalam database"));
            } else {
                QMessageBox::information(nullptr, tr("Error"), err.text());
            }
        }
        QMessageBox::information(nullptr, tr("Error"), tr("Gagal mengeksekusi query"));
        q.exec("ROLLBACK");
        return;
    }
    q.exec("COMMIT;");
    auto q = QMessageBox::question(nullptr, tr("Buat lagi?"), tr("Lanjutkan menambahkan data konsumen?"));
    if(q == QMessageBox::Yes){
        reset_input();
    }
};

void CreateCustomerDialog::reset_input() {
    hide();
    ui->namaLineEdit->clear();
    ui->alamatLineEdit->clear();
    ui->nomorTeleponLineEdit->clear();
    show();
}