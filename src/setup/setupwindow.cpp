#include "setupwindow.h"
#include "ui_setupwindow.h"

#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

SetupWindow::SetupWindow(QWidget *p) :
  ui(new Ui::SetupWindow), QMainWindow(p)
{
  ui->setupUi(this);
}

SetupWindow::~SetupWindow()
{
  delete ui;
}

void SetupWindow::on_browseButton_clicked() 
{
  auto databaseDir = QFileDialog::getExisitingDirectory(this, "Pilih penyimpanan database");
  if (databaseDir.isEmpty()) return;
  ui->lineEdit->setText(databaseDir);
}

void SetupWindow::on_installButton_clicked() 
{
  QMap<QString, QString> params = {
    { "dbDir"        ,ui->lineEdit->text() },
    { "compName"     ,ui->nameEdit->text() },
    { "compTelp"     ,ui->telpEdit->text() },
    { "compEmail"    ,ui->emailEdit->text() },
    { "compAddress"  ,ui->addressEdit->toPlainText() },
    { "username"     ,ui->usernameEdit->text() },
    { "password"     ,ui->passwordEdit->text() },
    { "rePassword"   ,ui->rePasswordEdit->text() },
    { "fullname"     ,ui->fullnameEdit->text() },
    { "telpUser"     ,ui->telpUserEdit->text() },
    { "addrUser"     ,ui->addressUserEdit->toPlainText()}
  };
  
  auto dmesg = [this](QString t, QString m) {
    QMessageBox::information(this, t, m); 
  }
  
  if (params["dbDir"].isEmpty()) {
    dmesg("Informasi", "Anda belum menentukan tempat penyimpanan database");
    return;
  }

  if (params["compName"].isEmpty()) {
    dmesg("Informasi", "Anda belum mengisi nama perusahaan");
    return;
  }

  if (params["compTelp"].isEmpty()) {
    dmesg("Informasi", "Anda belum mengisi nomor telepon perusahaan");
    return;
  }
  
  if (params["compEmail"].isEmpty()) {
    dmesg("Informasi", "Anda belum mengisi alamat email perusahaan");
    return;
  }
  
  if (params["compAddress"].isEmpty()) {
    dmesg("Informasi", "Anda belum menentukan alamat perusahaan");
    return;
  }
  
  if (params["username"].isEmpty()) {
    dmesg("Informasi", "Anda belum mengisi nama super admin");
    return;
  }
  
  if (params["password"].isEmpty()) {
    dmesg("Informasi", "Anda belum memberikan katasandi");
    return;
  }
  
  if (params["password"].length() < 6) {
    dmesg("Informasi", "Katasandi terlalu pendek, masukan minimal 6 karakter");
    return;
  }
  
  if (params["rePassword"] != params["password"] ) {
    dmesg("Informasi", "Katasandi pertama dan kedua tidak cocok");
    return;
  }
  
  if (params["fullname"].isEmpty()) {
    dmesg("Informasi", "Nama lengkap belum diisi");
    return;
  }
  
  if (params["telpUser"].isEmpty()) {
    dmesg("Informasi", "Nomor telepon user belum diisi");
    return;
  }
  
  if (params["addrUser"].isEmpty()) {
    dmesg("Informasi", "Alamat user belum diisi");
    return;
  }
  
  if (QDir(params["dbDir"]).exists("LAdminDB.dat")) {
    dmesg("Folder tidak kosong", QString("Folder berisikan file database, tidak dapat menggunakan folder ini :\n%1").arg(QDir(params["dbDir"]).absoluteFilePath("LAdminDB.dat")));
    ui->lineEdit->clear();
    return;
  }
  
  
}