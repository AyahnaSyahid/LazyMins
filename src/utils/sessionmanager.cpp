#include "src/utils/sessionmanager.h"
#include "src/utils/authmanager.h"
#include "src/managers/adminmanager.h"
#include "sessionmanager.h"

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
    m_currentUserRecord = orc;
    emit loginSuccess();
    emit userChanged();
    return ;
  }
  emit loginFailed("Nama dan sandi anda TIDAK COCOK");
}

void SessionManager::logout() {
  if (m_currentUserRecord.has_value()) {
    AdminManager am;
    am.setLastLog(m_currentUserRecord);
    m_currentUserRecord = std::nullopt;
  }
  emit userLogout();
}

std::optional<QSqlRecord> SessionManager::currentUser() const {
  return m_currentUserRecord;
}

bool SessionManager::currentUserPasswordMatch(const QString& pass) const {
  if(!m_currentUserRecord) return false;
  QString username = (*m_currentUserRecord).value("username").toString();
  AdminManager am;
  return am.passwordMatch(username, pass);
}

bool SessionManager::isSuperAdminSession() const { 
  AdminManager am;
  if(!m_currentUserRecord) return false;
  return am.userHasRole(m_currentUserRecord->value("id").toInt(), "super_admin");
}

int SessionManager::currentUserId() const
{
  if(!m_currentUserRecord) return -1;
  return (*m_currentUserRecord).value("id").toInt();
}
