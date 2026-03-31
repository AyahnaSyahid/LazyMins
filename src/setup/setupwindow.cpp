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
  
  QStringList errors;
  if(params["dbDir"].isEmpty())
    errors << "- Tempat penyimpanan database";
  if(params["compName"].isEmpty())
    errors << "- Nama perushaan";
  if(params["compTelp"].isEmpty())
    errors << "- Nomor telepon perushaan";
  if(params["compEmail"].isEmpty())
    errors << "- EMail perushaan";
  if(params["compAddress"].isEmpty())
    errors << "- Alamat Perusahaan / Toko";
  if(params["username"].isEmpty())
    errors << "- Nama Super Admin";
  if(params["password"].isEmpty())
    errors << "- Password";
  if(params["rePassword"].isEmpty())
    errors << "- Verifikasi Password";
  if(params["fullname"].isEmpty())
    errors << "- Nama lengkap user";
  if(params["telpUser"].isEmpty())
    errors << "- Nomor telepon user";
  if(params["addrUser"].isEmpty())
    errors << "- Alamat / Domisili user";
  
  if(errors.size() > 0) {
    dmesg("Semua field diperlukan", QString("Pastikan untuk mengisi field berikut :\n%1").arg(errors.join("\n")));
    return ;
  }
  errors.clear();
  
  if (params["password"].size() < 6) {
    dmesg("Tinjau input password", "Jumlah karakter Password kurang dari 6 karakter");
    return;
  }
  
  if (params["password"].size() < 6) {
    dmesg("Tinjau verifikasi password", "Verifikasi password tidak cocok");
    return;
  }
  
  // all done
  
}