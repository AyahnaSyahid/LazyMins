#include "createuserdialog.h"
#include "ui_createuserdialog.h"

#include "src/database/databasemanager.h"
#include "src/managers/adminmanager.h"

#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>

CreateUserDialog::CreateUserDialog(QWidget *p) :
ui (new Ui::CreateUserDialog), qmodel(new QSqlQueryModel(this)), QDialog(p) 
{
  ui->setupUi(this);
  qmodel->setQuery("SELECT id AS ID, role_name AS Peran, description AS Keterangan FROM roles", 
    DatabaseManager::instance().database());
  ui->roleBox->setModel(qmodel);
  ui->roleBox->setModelColumn(1);
  
  auto boxView = new QTableView();
  ui->roleBox->setView(boxView);
  ui->roleBox->setCurrentIndex(2);
  
  auto vh = boxView->verticalHeader();
  vh->setMinimumSectionSize(22);
  vh->setDefaultSectionSize(20);
  vh->hide();
  boxView->horizontalHeader()->hide();
  boxView->hideColumn(0);
  boxView->setAlternatingRowColors(true);
  boxView->resizeColumnsToContents();
  boxView->setHorizontalScrollMode(QTableView::ScrollPerPixel);
  boxView->setSelectionBehavior(QTableView::SelectRows);
  boxView->setMinimumWidth(boxView->columnWidth(1) + boxView->columnWidth(2));
}

CreateUserDialog::~CreateUserDialog() { delete ui; }

bool CreateUserDialog::checkInputs() {
  QList<QLineEdit*> lineEdits { 
      ui->fullnameEdit, 
      ui->phoneEdit,
      ui->emailEdit,
      ui->usernameEdit,
      ui->passwordEdit1,
      ui->passwordEdit2 };
  
  for(auto *edit : lineEdits) {
    if (edit->text().isEmpty()) {
      edit->setStyleSheet("background-color: rgb(255, 240, 181);");
      QMessageBox::information(this, "Periksa Input", "Semua field harus diisi");
      edit->setFocus();
      edit->selectAll();
      return false;
    } else {
      edit->setStyleSheet("");
    }
  }
  
  if (ui->passwordEdit1->text() != ui->passwordEdit2->text()) {
    ui->passwordEdit2->setStyleSheet("background-color: rgb(255, 240, 181);");
    QMessageBox::information(this, "Periksa Password", "Verifikasi password yang anda masukkan tidak sesuai dengan field password diatasnya");
    ui->passwordEdit2->setFocus();
    ui->passwordEdit2->selectAll();
    return false;
  } else {
    ui->passwordEdit2->setStyleSheet("");
  }
  return true;
}

void CreateUserDialog::on_simpanButton_clicked() {
  if (!checkInputs()) return;
  AdminManager am;
  
  if(am.exists(ui->usernameEdit->text())) {
    QMessageBox::information(this, "Maaf", "Username ini telah digunakan admin lain\nCoba gunakan Username yang lain");
    return ;
  }
  
  QVariantMap param {
    { "username",         QVariant(ui->usernameEdit->text()) },
    { "role_id",          QVariant(qmodel->index(ui->roleBox->currentIndex(), 0).data(Qt::EditRole).toInt()) },
    { "literal_password", QVariant(ui->passwordEdit1->text()) },
    { "nama_lengkap",     QVariant(ui->fullnameEdit->text()) },
    { "email",            QVariant(ui->emailEdit->text()) },
    { "nomor_telp",       QVariant(ui->phoneEdit->text()) },
    { "is_active",        QVariant(true)} 
  };
  
  auto oprec = am.create(param);
  if (oprec) {
    emit userAdded();
    accept();
    return ;
  }
  
  QMessageBox::information(this, "Kesalahan", "Gagal menambahkan user kedalam database");
};