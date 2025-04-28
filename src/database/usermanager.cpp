#include "usermanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QtDebug>
#include <QRandomGenerator>
#include <QVariant>

#ifndef HASH_ITERATION_COUNT
#define HASH_ITERATION_COUNT 98975
#endif

#ifndef RANDCHAR_CHOICES
#define RANDCHAR_CHOICES "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
#endif

#ifndef SALT_CHAR_LENGTH
#define SALT_CHAR_LENGTH 32
#endif

UserManager::UserManager(QObject* parent) : QObject(parent), _c_id(0)
{
    QSqlQuery q("SELECT salt FROM users WHERE account_name = 'admin'");
    if(!q.next()) {
        createUser("admin", "admin", "Admin");
    }
}

UserManager::~UserManager()
{
    logout();
}

bool UserManager::login(const QString& acc, const QString& pw) {
    QSqlQuery q;
    q.prepare("SELECT user_id, account_name, password_hash, salt FROM users WHERE account_name = ?");
    q.addBindValue(acc);
    q.exec();
    if(!q.next()) {
        return false;
    }
    QByteArray hash = q.value("password_hash").toByteArray();
    QString salt = q.value("salt").toString();
    QByteArray calculated = generateHash(pw, salt);

    if(hash == calculated) {
        _c_id = q.value("user_id").toInt();
        emit userLoggedIn(_c_id);
        return true;
    }
    return false;
}

bool UserManager::nameExists(const QString& name) {
    QSqlQuery q;
    q.prepare("SELECT user_id FROM users WHERE account_name = ?");
    q.addBindValue(name.trimmed());
    q.exec();
    return q.next();
}

bool UserManager::createUser(
    const QString& acc,
    const QString& pw, 
    const QString& disp,
    QString* err,
    int* out_id )
{
    QString salt = generateSalt();
    QByteArray hash = generateHash(pw, salt);
    QSqlQuery q;
    q.prepare("INSERT INTO users (account_name, password_hash, salt, display_name) VALUES (?, ?, ?, ?); ");
    q.addBindValue(acc);
    q.addBindValue(hash);
    q.addBindValue(salt);
    q.addBindValue(disp);
    if(!q.exec()) {
        if(err) {
            *err = q.lastError().text();
        }
        if(out_id) {
            *out_id = 0;
        }
        return false;
    }
    if(err) *err = "";
    if(out_id) *out_id = q.lastInsertId().toInt();
    emit userCreated(q.lastInsertId().toInt());
    return true;
}

QString UserManager::generateSalt(int length) {
    int l = length == 0 ? SALT_CHAR_LENGTH : length;
    auto sys = QRandomGenerator::system();
    QString rc(RANDCHAR_CHOICES), buffer;
    while(--l) {
        auto ix = sys->bounded(rc.count());
        buffer.append(rc.at(ix));
    }
    return buffer;
}

QByteArray UserManager::generateHash(const QString& pw, const QString& salt) {
    QCryptographicHash hash(QCryptographicHash::Keccak_512);
    QByteArray combination = QString("%1::%2").arg(pw, salt).toUtf8();
    int i = HASH_ITERATION_COUNT;
    for(int a = 0; a < i; ++a) {
        hash.addData(combination);
    }
    return hash.result();
}

void UserManager::logout()
{
    QSqlQuery q;
    q.prepare("UPDATE users SET last_active = CURRENT_TIMESTAMP WHERE user_id = ?");
    q.addBindValue(_c_id);
    if( !q.exec() ) {
        qDebug() << q.lastError().text();
    }
    _c_id = 0;
    emit userLoggedOut();
};