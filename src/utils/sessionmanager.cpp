#include "src/utils/sessionmanager.h"
#include "src/utils/authmanager.h"
#include "src/managers/adminmanager.h"
#include <QMessageBox>

SessionManager &SessionManager::instance() {
  static SessionManager sm;
  return sm;
}

void SessionManager::login(const QString& name, const QString& pass) {
  emit userLogin();
  auto &auth = AuthManager::instance();
  // qDebug() << __FILE__ << name << pass;
  if(auth.passwordMatch(name, pass)) {
    AdminManager am;
    auto orc = am.getRecord(name);
    if (orc) {
      auto rc = *orc;
      if (!rc.value("is_active").toBool()) {
        // tidak aktif
        QMessageBox::warning(nullptr, "Peringatan", "Akun anda sedang berada dalam status PASIF");
        emit loginFailed();
        return ;
      }
    }
    m_optUserRecord = orc;
    emit loginSuccess();
    emit userChanged();
    return ;
  }
  QMessageBox::warning(nullptr, "Peringatan", "Nama dan sandi anda TIDAK COCOK");
  emit loginFailed();
}

void SessionManager::logout() {
  emit userLogout();
  m_optUserRecord = std::nullopt;
  emit userChanged();
}

std::optional<QSqlRecord> SessionManager::currentUser() const {
  return m_optUserRecord;
}