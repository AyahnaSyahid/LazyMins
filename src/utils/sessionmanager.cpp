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
  if (m_optUserRecord.has_value()) {
    AdminManager am;
    am.setLastLog(m_optUserRecord);
    m_optUserRecord = std::nullopt;
  }
  emit userLogout();
}

std::optional<QSqlRecord> SessionManager::currentUser() const {
  return m_optUserRecord;
}

bool SessionManager::currentUserPasswordMatch(const QString& pass) const {
  if(!m_optUserRecord) return false;
  QString username = (*m_optUserRecord).value("username").toString();
  auto &auth = AuthManager::instance();
  return auth.passwordMatch(username, pass);
}
