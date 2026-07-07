#pragma once

#include <QObject>
#include <QSqlRecord>
#include <QEvent>

#ifndef SESSION_MAX_IDLE_TIME
#define SESSION_MAX_IDLE_TIME 300'000 // 5 minutes
#endif

class QTimer;
class SessionManager : public QObject
{
  Q_OBJECT

public:
  static SessionManager &instance();
  std::optional<QSqlRecord> currentUser() const;
  bool currentUserPasswordMatch(const QString& ) const;
  bool isSuperAdminSession() const;
  int currentUserId() const;

public slots:
  void login(const QString &name, const QString &password);
  void logout();
  void restartIdleTimer();

signals:
  void userChanged();
  void userLogin();
  void userLogout();
  void loginFailed(const QString&);
  void loginSuccess();
  void idleTimeout();
  void locked(); // Signal baru untuk menandai aplikasi terkunci

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  SessionManager();
  ~SessionManager();
  std::optional<QSqlRecord> m_currentUserRecord;
  QTimer *m_idleTimer;
};
