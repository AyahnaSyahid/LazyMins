#include "usermanager.h"
#include <QSqlQuery>
#include <QCryptographicHash>
#include <QDateTime>
#include <QByteArray>

#ifndef HASH_ITERATION_COUNT
#define HASH_ITERATION_COUNT 98975
#endif

#ifndef RANDCHAR_CHOICES
#define RANDCHAR_CHOICES "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
#endif

#ifndef SALT_CHAR_LENGTH
#define SALT_CHAR_LENGTH 8
#endif

UserManager::UserManager(QObject* parent) : QObject(parent), _c_id(0) {}
UserManager::~UserManager()
{
    logout();
}

bool UserManager::login(const QString& acc, const QString& pw) {}

bool UserManager::createUser(
    const QString& acc,
    const QString& pw, 
    const QString& disp,
    QString* err,
    int* out_id )
{
    if(out_id) out_id = 0;
    
    QSqlQuery q;
    q.prepare("SELECT user_id FROM users WHERE account_name = ?");
    q.addBindValue(acc);
    q.exec();
    if(q.next()) {
        if(err) {
            err = tr("Nama akun telah digunakan, coba dengan nama lain");
        }
        return false;
    }
    if(pw.isEmpty()) {
        if(err) {
            err = tr("Password kosong, demi keamanan mohon tambahkan password");
        }
        return false;
    }
    QString randStrings;
    QString charChoices(RANDCHAR_CHOICES);
    for(int i=0; i<SALT_CHAR_LENGTH; ++i) {
        int ix = QRandomGenerator::sistem()->bounded(charChoices.count());
        randStrings.append(charChoices.at(ix));
    }
    QString nameAndPw = QString("%1::%2").arg(acc, pw);
    QByteArray hash = QCryptographicHash::hash(nameAndPw.toUtf8());
    int iteration = HASH_ITERATION_COUNT;
    while(iteration--) {
        hash.addData(randStrings.toUtf8());
    }
        
}

QByteArray UserManager::generateHash(const QString& name_pw) {
    QCryptographicHash hash(QCryptographicHash::SHA_256);
    hash.addData(name_pw.toUtf8());
    int i = HASH_ITERATION_COUNT;
    while(i--) {
        hash.addData()
    }
}

void UserManager::logout()
{
    // direct update last_active
    QSqlQuery q(QString("UPDATE users SET last_active = CURRENT_TIMESTAMP WHERE user_id = %1").arg(_c_id));
    _c_id = 0;
    emit userLoggedOut();
};