#include "logindialog.h"

#include <QMessageBox>
#include <QTimer>
#include <QCloseEvent>

#include "src/managers/appsettingsmanager.h"
#include "src/customs/buttonguard.h"
#include "src/utils/sessionmanager.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *p)
    : QDialog(p), m_failCount(3), ui(new Ui::LoginDialog)
{
  ui->setupUi(this);
  auto &sm = SessionManager::instance();

  connect(&sm, &SessionManager::loginSuccess, this, &QDialog::accept);
  connect(&sm, &SessionManager::loginFailed, this,
          &LoginDialog::decrementChances);

  reenableLogin();
}

LoginDialog::~LoginDialog() { delete ui; }

void LoginDialog::on_masukButton_clicked()
{
  ButtonGuard guard(ui->masukButton);
  QString name = ui->nameEdit->text(), pass = ui->passEdit->text();

  if (name.isEmpty() || pass.isEmpty())
  {
    QMessageBox::information(this, "Kesalahan", "Semua field harus diisi");
    return;
  }
  auto &sm = SessionManager::instance();
  sm.login(name, pass);
}

void LoginDialog::decrementChances()
{
  if (--m_failCount <= 0)
  {
    ui->masukButton->setEnabled(false);
    QMessageBox::warning(this, "Silahkan Menunggu",
                         "Anda telah gagal masuk beberapa kali, \nmohon "
                         "menunggu sampai tombol masuk kembali aktif");
    auto apm = AppSettingsManager();
    auto timeout = apm.getSettings("login_failCount_timeout_sec");
    if (!timeout.isEmpty())
    {
      QTimer::singleShot(timeout.value("setting_value").toInt() * 1000, this,
                         &LoginDialog::reenableLogin);
    }
    else
    {
      QTimer::singleShot(3000, this, &LoginDialog::reenableLogin);
    }
    return;
  }
}

void LoginDialog::reenableLogin()
{
  auto apm = AppSettingsManager();
  auto smap = apm.getSettings("max_login_failCount");
  m_failCount = smap.isEmpty() ? 3 : smap.value("setting_value").toInt();
  ui->masukButton->setEnabled(true);
}

void LoginDialog::closeEvent(QCloseEvent *evt)
{
  if (SessionManager::instance().currentUserId() > 0)
  {
    evt->accept();
    return;
  }

  // PENTING: closeEvent() di sini bisa dipicu langsung oleh window manager
  // (tombol X native), yaitu SINKRON di dalam handler event sistem
  // (QGuiApplicationPrivate::processCloseEvent -> handleClose -> closeEvent).
  // Membuka QMessageBox modal langsung di titik ini pernah menyebabkan
  // crash, karena native window sedang dalam proses ditutup oleh WM/portal
  // saat dialog native baru dibuka di atasnya (race condition pada
  // QDialogPrivate::setNativeDialogVisible/hide()).
  //
  // Solusi: SELALU abaikan close event native ini dulu, lalu jadwalkan
  // konfirmasi untuk dijalankan di iterasi event loop berikutnya, di luar
  // call stack native close handler.
  evt->ignore();

  if (m_confirmingExit)
    return; // sudah ada konfirmasi yang sedang/akan berjalan, jangan dobel

  QTimer::singleShot(0, this, &LoginDialog::confirmAndExit);
}

void LoginDialog::confirmAndExit()
{
  if (m_confirmingExit)
    return;

  m_confirmingExit = true;
  setEnabled(false);

  auto btn = QMessageBox::question(
      this, "Batal Masuk",
      "Anda yakin membatalkan masuk ?\nIni akan menutup aplikasi",
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  m_confirmingExit = false;

  if (btn == QMessageBox::Yes)
  {
    // Tidak perlu lagi accept()/reject() window secara "graceful": kita
    // langsung sembunyikan dan keluarkan seluruh aplikasi. Ini juga
    // menghindari perlu memicu closeEvent() lagi (yang berarti tidak perlu
    // masuk lagi ke jalur native close event yang bermasalah).
    hide();
    QTimer::singleShot(100, qApp, &QCoreApplication::quit);
    return;
  }

  setEnabled(true);
}

void LoginDialog::reject()
{
  if (SessionManager::instance().currentUserId() > 0)
  {
    QDialog::reject();
  }
  else
  {
    // Esc ditangani oleh QDialog secara normal (bukan lewat native
    // close-event WM), jadi aman menjadwalkan dengan cara yang sama demi
    // konsistensi dan supaya guard m_confirmingExit tetap tunggal.
    if (m_confirmingExit)
      return;
    QTimer::singleShot(0, this, &LoginDialog::confirmAndExit);
  }
}

void LoginDialog::setUsername(const QString &name)
{
  ui->nameEdit->setText(name);
}
