#include "createcustomerdialog.h"
#include "files/ui_createcustomerdialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QPushButton>

CreateCustomerDialog::CreateCustomerDialog(QWidget* parent)
: ui(new Ui::CreateCustomerDialog), QDialog(parent) {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
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
    
    QSqlQuery q("BEGIN TRANSACTION;");
    q.prepare("INSERT INTO customers (name, address, phone) VALUES (?, ?, ?)");
    q.addBindValue(nama);
    q.addBindValue(alm);
    q.addBindValue(telp);
    if(!q.exec()) {
        auto err = q.lastError();
        if(err.isValid()) {
            if(err.text().contains("Unique_ID_Customer", Qt::CaseInsensitive)) {
                QMessageBox::information(this, tr("Duplikasi"), tr("Konsumen dengan nama, alamat dan nomor telepon ini telah tersimpan dalam database"));
            } else {
                QMessageBox::information(this, tr("Error"), err.text());
            }
        }
        QMessageBox::information(this, tr("Error"), tr("Gagal mengeksekusi query"));
        q.exec("ROLLBACK");
        show();
        return;
    }
    q.exec("COMMIT;");
    QMessageBox queBox = QMessageBox(QMessageBox::Question, tr("Buat lagi?"), tr("Lanjutkan menambahkan data konsumen?"), QMessageBox::Yes | QMessageBox::No, this);
    queBox.button(QMessageBox::Yes)->setText("Ya");
    queBox.button(QMessageBox::No)->setText("Tidak");
    
    auto que = queBox.exec();
    if(que == QMessageBox::Yes){
        reset_input();
        emit accepted();
        show();
        return;
    }
    accept();
};

void CreateCustomerDialog::reset_input() {
    ui->namaLineEdit->clear();
    ui->alamatLineEdit->clear();
    ui->nomorTelpLineEdit->clear();
}