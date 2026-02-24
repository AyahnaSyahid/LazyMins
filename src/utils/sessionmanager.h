#pragma once

#include <QObject>

class SessionManager : public QObject
{
  Q_OBJECT

public:
  static SessionManager &instance();

public slots:
  void login(const QString &name, const QString &password);
  void logout();
  std::optional<QSqlRecord> &currentUser() const;
  
  
signals:
  void userChanged();
  void userLogin();
  void userLogout();
  void loginFailed();

private:
  SessionManager() : QObject(nullptr) {}
  std::optional<QSqlRecord> m_optUserRecord;
};
