#include "config.h"
#include "loginnotifier.h"
#include "usermanager.h"
#include <QMutexLocker>
#include <QtDebug>

LoginNotifier* LoginNotifier::ln = nullptr;
QMutex LoginNotifier::smutex;

LoginNotifier* LoginNotifier::instance(QObject *parent) {
    // QMutexLocker locker(&smutex);
    if(!ln) {
        ln = new LoginNotifier(parent);
    }
    return ln;
}

const UserItem LoginNotifier::currentUser() {
    return UserItem(GlobalNamespace::logged_user_id);
}

LoginNotifier::LoginNotifier(QObject* parent)
   : QObject(parent)
{}

LoginNotifier::~LoginNotifier()
{
    qDebug() << objectName() << "deleted";
}

void LoginNotifier::setLoggedUser(qint64 userid)
{
    QMutexLocker locker(&mutex);
    if(GlobalNamespace::logged_user_id > 0) {
        logout();
    }
    GlobalNamespace::logged_user_id = userid;
    // locker.unlock();
    emit userLoggedIn(userid);
}

void LoginNotifier::logout() {
    QMutexLocker locker(&mutex);
    auto lastUser = GlobalNamespace::logged_user_id;
    if(lastUser) {
        GlobalNamespace::logged_user_id = 0;
        UserManager().setLastActive(lastUser);
        emit userLoggedOut(lastUser);
    }
}