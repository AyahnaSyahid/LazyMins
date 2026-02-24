#include "src/utils/sessionmanager.h"
#include "src/utils/authmanager.h"
#include "src/managers/adminmanager.h"

SessionManager &SessionManager::instance() {
  static SessionManager sm;
  return sm;
}

void SessionManager::login(const QString& name, const QString& pass) {
  emit userLogin();
  auto &auth = AuthManager::instance();
  if(auth.passwordMatch(name, pass)) {
    AdminManager am;
    m_optUserRecord = am.getRecord(name);
    emit userChanged();
    return ;
  }
  emit loginFailed();
}

void SessionManager::logout() {
  emit userLogout();
  m_optUserRecord = std::nullopt;
  emit userChanged();
}

std::optional<QSqlRecord> &currentUser() const {
  return m_optUserRecord;
}