#pragma once

#include <QObject>
#include <QSqlRecord>

class SessionManager : public QObject
{
  Q_OBJECT

public:
  static SessionManager &instance();
  std::optional<QSqlRecord> currentUser() const;
  bool currentUserPasswordMatch(const QString& ) const;

public slots:
  void login(const QString &name, const QString &password);
  void logout();

signals:
  void userChanged();
  void userLogin();
  void userLogout();
  void loginFailed();
  void loginSuccess();

private:
  SessionManager() : QObject(nullptr) {}
  std::optional<QSqlRecord> m_optUserRecord;
};
