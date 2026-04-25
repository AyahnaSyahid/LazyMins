#include "src/utils/sessionmanager.h"
#include "src/utils/authmanager.h"
#include "src/managers/adminmanager.h"

SessionManager &SessionManager::instance() {
  static SessionManager sm;
  return sm;
}

void SessionManager::login(const QString& name, const QString& pass) {
  emit userLogin();
  AdminManager am;
  if(am.passwordMatch(name, pass)) {
    auto orc = am.getRecord(name);
    if (orc) {
      auto rc = *orc;
      if (!rc.value("is_active").toBool()) {
        emit loginFailed("Akun anda sedang berada dalam status PASIF");
        return ;
      }
    }
    m_optUserRecord = orc;
    emit loginSuccess();
    emit userChanged();
    return ;
  }
  emit loginFailed("Nama dan sandi anda TIDAK COCOK");
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
  AdminManager am;
  return am.passwordMatch(username, pass);
}
