#include "konfigurasi.h"
#include "ui_konfigurasi.h"

#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>

Konfigurasi::Konfigurasi(QWidget *p) :
  ui(new Ui::Konfigurasi), QDialog(p)
{
  ui->setupUi(this);
  QSettings s;
  ui->dbFile->setText(s.value("Database/Path").toString());
  ui->sPort->setText(s.value("pos_printer/serial_port").toString());
}

Konfigurasi::~Konfigurasi() { delete ui; }

void Konfigurasi::on_pilihButton_clicked() {
  QString saveName = QFileDialog::getSaveFileName(
    this, "Pilih penyimpanan Database", QDir::homePath());
  ui->dbFile->setText(saveName);
}

void Konfigurasi::accept() {
  if (ui->dbFile->text().isEmpty()) {
    QMessageBox::information(this, "Kesalahan", "Anda belum menentukan tempat penyimpanan database");
    return ;
  }
  if (ui->sPort->text().isEmpty()) {
    QMessageBox::information(this, "Kesalahan", "Anda belum menentukan port printer yang akan digunakan");
    return;
  }
  
  QSettings s;
  s.setValue("Database/Path", ui->dbFile->text());
  s.setValue("pos_printer/serial_port", ui->sPort->text());
  s.sync();
  if (s.status() != QSettings::NoError) {
    QMessageBox::information(this, "Kesalahan", "Tidak dapat menyimpan konfigurasi");
    return ;
  }
  QDialog::accept();
}
