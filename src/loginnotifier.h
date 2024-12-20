#ifndef LoginNotifierH
#define LoginNotifierH

#include <QObject>
#include <QMutex>
#include "useritem.h"


class LoginNotifier : public QObject
{
    Q_OBJECT
    static LoginNotifier* ln;
    explicit LoginNotifier(QObject* = nullptr);
    static QMutex smutex;
    QMutex mutex;
  public:
    ~LoginNotifier();
     static LoginNotifier* instance(QObject* =nullptr);
     static const UserItem currentUser();
     static bool passwordVerification(const UserItem&);
     LoginNotifier(const LoginNotifier&) = delete;
     LoginNotifier& operator=(const LoginNotifier&) = delete;

  public slots:
    void setLoggedUser(qint64 userid);
    void logout();

  signals:
    void userLoggedIn(qint64 userId);
    void userLoggedOut(qint64 userId);
};

#define logNot LoginNotifier::instance()

#endif //LoginNotifierH