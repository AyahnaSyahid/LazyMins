#include "usermanager.h"
#include <QSqlQuery>
#include <QCryptographicHash>

#ifndef HASH_ITERATION_COUNT
#define HASH_ITERATION_COUNT 98975
#endif

UserManager::UserManager(QObject* parent) : QObject(parent), _c_id(0) {}
UserManager::~UserManager()
{
    logout();
}

bool login(const QString& acc, const QString& pw) {}

bool createUser(const QString& acc,
                const QString& pw, 
                const QString& disp,
                QString* err,
                int* out_id)
{
    if(out_id) out_id = 0;
    
    QSqlQuery q;
    q.prepare("SELECT user_id FROM users WHERE account_name = ?");
    q.addBindValue(acc);
    q.exec();
    if(q.next()) {
        if(err) {
            err = "User already exists, try another account_name";
        }
        return false;
    }
    
}

void logout()
{
    // direct update last_active
    QSqlQuery q(QString("UPDATE users SET last_active = CURRENT_TIMESTAMP WHERE user_id = %1").arg(_c_id));
    _c_id = 0;
    emit userLoggedOut();
};