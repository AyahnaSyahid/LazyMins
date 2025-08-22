#include "usermanager.h"
#include "passworddialogs.h"
#include "files/ui_changepassworddialog.h"
#include "files/ui_revokepassworddialog.h"
#include "files/ui_userselectordialog.h"
#include <QSqlTableModel>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QMessageBox>
#include <QTimer>
#include <QtDebug>

ChangePasswordDialog::ChangePasswordDialog(UserManager* _uman, QWidget* parent)
: uman(_uman), ui(new Ui::ChangePasswordDialog()), QDialog(parent) {
  ui->setupUi(this);
}

ChangePasswordDialog::~ChangePasswordDialog() {}

void ChangePasswordDialog::on_saveButton_clicked() {
  QString crPass = ui->currentPassword->text(),
          nwPass = ui->newPassword->text(),
          retype = ui->retypePassword->text();
  if(crPass.isEmpty()) {
    QMessageBox::information(this, "Periksa input", "Password lama belum diisi");
    ui->currentPassword->setFocus(Qt::OtherFocusReason);
    return ;
  } else if(nwPass.isEmpty()){
    QMessageBox::information(this, "Periksa input", "Kata Sandi baru belum di setel");
    ui->newPassword->setFocus(Qt::OtherFocusReason);
    return ;
  } else if(retype.isEmpty()) {
    QMessageBox::information(this, "Periksa input", "Ulangi password baru");
    ui->retypePassword->setFocus(Qt::OtherFocusReason);
    return ;
  }
  int loggedUserId = uman->currentUser();
  QString crName = UserManager::getNameById(loggedUserId);
  if(!UserManager::nameAndPasswordMatch(crName, crPass)) {
    QMessageBox::information(this, "Password Salah", "Pastikan anda mengetikkan password lama anda dengan benar");
    return ;
  }
  
  if(nwPass != retype) {
    QMessageBox::information(this, "Password Salah", "Password baru dan verifikasi password baru tidak sama");
    return ;
  }
  
  if(uman->changePassword(nwPass)) {
    QMessageBox::information(this, "Sukses", "Password berhasil diganti");
    accept();
  }

}

ChangeOtherPasswordDialog::ChangeOtherPasswordDialog(int userId, UserManager* _uman, QWidget* parent)
: euid(userId), ChangePasswordDialog(_uman, parent) {
  ui->groupBox->setTitle(QString("Setel Password untuk Akun : %1").arg(UserManager::getNameById(euid)));
  ui->label->hide();
  ui->currentPassword->hide();
  ui->line->hide();
  adjustSize();
}

ChangeOtherPasswordDialog::~ChangeOtherPasswordDialog(){}

void ChangeOtherPasswordDialog::revokePassword() {
  auto rpd = new RevokePasswordDialog(uman, this);
  connect(rpd, &QDialog::rejected, this, &QDialog::reject);
  connect(rpd, &RevokePasswordDialog::revokeSuccess, this, &QDialog::open);
  rpd->setAttribute(Qt::WA_DeleteOnClose);
  rpd->open();
}

void ChangeOtherPasswordDialog::on_saveButton_clicked() {
  QString np = ui->newPassword->text(),
          rp = ui->retypePassword->text();
  if(np.count() < 5) {
    QMessageBox::information(this, "Tidak memenuhi syarat", "Password harus terdiri dari 5 karakter atau lebih");
    return ;
  }
  if(np != rp) {
    QMessageBox::information(this, "Tidak Cocok", "Sandi dan pengulang sandi tidak cocok");
    return ;
  }
  uman->changePassword(euid, np);
  accept();
}

RevokePasswordDialog::RevokePasswordDialog(UserManager *_uman, QWidget *parent)
: uman(_uman), ui(new Ui::RevokePasswordDialog), QDialog(parent) {
  ui->setupUi(this);
}

RevokePasswordDialog::~RevokePasswordDialog() {}

void RevokePasswordDialog::on_okButton_clicked() {
  if(uman->revokePassword(ui->revoked->text())) {
    emit revokeSuccess();
    accept();
  }
}

UserSelectorDialog::UserSelectorDialog(int selid, QWidget* parent)
: ui(new Ui::UserSelectorDialog), sid(-1), QDialog(parent) {
  ui->setupUi(this);
  auto m = new QSqlTableModel(this);
  m->setTable("users");
  m->setFilter(QString("user_id != %1").arg(selid));
  m->select();
  m->setHeaderData(1, Qt::Horizontal, "Nama Akun", Qt::DisplayRole);
  m->setHeaderData(4, Qt::Horizontal, "Nama Display", Qt::DisplayRole);
  ui->userView->setModel(m);
  ui->userView->hideColumn(0);
  ui->userView->hideColumn(2);
  ui->userView->hideColumn(3);
  ui->userView->hideColumn(5);
  ui->userView->hideColumn(6);
  ui->userView->hideColumn(7);
  ui->userView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

UserSelectorDialog::~UserSelectorDialog() {}

void UserSelectorDialog::on_pilihButton_clicked() {
  auto sm = ui->userView->selectionModel();
  auto currentIndex = sm->currentIndex();
  if(!currentIndex.isValid()) {
    QMessageBox::information(this, "Pilih User", "Anda belum memilih User");
    return ;
  }
  sid = currentIndex.siblingAtColumn(0).data(Qt::EditRole).toInt();
  accept();
};