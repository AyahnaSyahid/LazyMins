#include "logindialog.h"
#include "ui_logindialog.h"

#include "src/managers/appsettingsmanager.h"
#include "src/utils/sessionmanager.h"

#include <QMessageBox>
#include <QTimer>

LoginDialog::LoginDialog(QWidget *p) :
  ui(new Ui::LoginDialog), m_failCount(3), QDialog(p)
{
  ui->setupUi(this);
  auto &sm = SessionManager::instance();
  
  connect(&sm, &SessionManager::loginSuccess, this, &QDialog::accept);
  connect(&sm, &SessionManager::loginFailed, this, &LoginDialog::decrementChances);
  
  reenableLogin();
}

LoginDialog::~LoginDialog () {delete ui;}
void LoginDialog::on_masukButton_clicked() {
  QString name = ui->nameEdit->text(),
          pass = ui->passEdit->text();
  
  if (name.isEmpty() || pass.isEmpty()) {
    QMessageBox::information(this, "Kesalahan", "Semua field harus diisi");
    return ;
  }
  auto &sm = SessionManager::instance();
  sm.login(name, pass);
}

void LoginDialog::decrementChances() {
  --m_failCount;
  if (m_failCount <= 0) {
    ui->masukButton->setEnabled(false);
    QMessageBox::warning(this, "Silahkan Menunggu", "Anda telah gagal masuk beberapa kali, \nmohon menunggu sampai tombol masuk kembali aktif");
    auto apm = AppSettingsManager();
    auto timeout = apm.getSettings("login_failCount_timeout_sec");
    if (!timeout.isEmpty()) {
      QTimer::singleShot(timeout.value("setting_value").toInt() * 1000, this, &LoginDialog::reenableLogin);
    } else {
      QTimer::singleShot(3000, this, &LoginDialog::reenableLogin);
    }
    return;
  }
}

void LoginDialog::reenableLogin() {
  auto apm = AppSettingsManager();
  auto smap = apm.getSettings("max_login_failCount");
  m_failCount = smap.isEmpty() ? 3 : smap.value("setting_value").toInt();
  ui->masukButton->setEnabled(true);
}