#include "usermanager.h"
#include "helper.h"
#include <QByteArray>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QtDebug>

QString UserManager::generateSalt(int length) const {
    // Generate a random salt
    QByteArray salt = QByteArray(length, '\0');
    for (int i = 0; i < length; ++i) {
        salt[i] = static_cast<char>(QRandomGenerator::global()->generate() % 256);
    }
    return salt.toBase64(); // Encode salt to Base64 for easy storage
}

QString UserManager::generatePasswordHash(const QString &password, const QString &salt, unsigned int iterations) const {
    QByteArray hash = password.toUtf8();
    QByteArray basalt = QByteArray::fromBase64(salt.toUtf8());
    hash += basalt;
    for (int i = 0; i < iterations; ++i) {
        hash = QCryptographicHash::hash(hash + basalt, QCryptographicHash::Sha256);
    }
    return hash.toHex(); // Return hex representation of the final hash
}

bool UserManager::createUser(const QString& uniqueName, const QString& password, qint64* id) {
    QSqlQuery q;
    q.prepare(R"(
        INSERT INTO users (username, salt, password_hash) 
            VALUES (?, ?, ?); )");
    QString rsalt = generateSalt(10);
    QString hash = generatePasswordHash(password, rsalt);
    q.addBindValue(uniqueName);
    q.addBindValue(rsalt);
    q.addBindValue(hash);
    if(q.exec()) {
        if(id) {
            *id = q.lastInsertId().toLongLong();
        }
        return true;
    } else {
        if(id) *id = -1;
        qDebug() << q.lastError().text();
    }
    return false;
}

bool UserManager::removeUser(const QString& username){
    QSqlQuery q;
    q.prepare("DELETE FROM users WHERE username = ?");
    q.addBindValue(username);
    q.exec();
    return q.numRowsAffected();
}

bool UserManager::userExists(const QString& name) const {
    QSqlQuery q;
    q.prepare("SELECT * FROM users WHERE username = ?");
    q.addBindValue(name);
    q.exec();
    return q.next();
}

bool UserManager::userExists(qint64 id) const {
    QSqlQuery q;
    q.prepare("SELECT * FROM users WHERE user_id = ?");
    q.addBindValue(id);
    q.exec();
    return q.next();
}

bool UserManager::passwordMatch(qint64 id, const QString& pw) const {
    QSqlQuery q;
    q.prepare("SELECT * FROM users WHERE user_id = ?");
    q.addBindValue(id);
    if(! (q.exec() && q.next()) ) return false;
    QString name = q.value("username").toString(),
            hash = q.value("password_hash").toString(),
            salt = q.value("salt").toString();
    return hash == generatePasswordHash(pw, salt);
}

bool UserManager::passwordMatch(const QString& username, const QString& pw) const {
    QSqlQuery q;
    q.prepare("SELECT * FROM users WHERE username = ?");
    q.addBindValue(username);
    if(! (q.exec() && q.next()) ) return false;
    QString name = q.value("username").toString(),
            hash = q.value("password_hash").toString(),
            salt = q.value("salt").toString();
    
    return hash == generatePasswordHash(pw, salt);
}

bool UserManager::changeUserPassword(qint64 id, const QString& _newp){
    QSqlQuery q;
    QString newSalt = generateSalt(), newHash;
    newHash = generatePasswordHash(_newp, newSalt);
    q.prepare("UPDATE users SET (salt, password_hash, mtime) = (?, ?, ?) WHERE user_id = ?");
    q.addBindValue(newSalt);
    q.addBindValue(newHash);
    q.addBindValue(StringHelper::currentDateTimeString());
    q.addBindValue(id);
    return q.exec() && q.numRowsAffected();
}

bool UserManager::changeUserPassword(const QString& uniqueName, const QString& _newp){
    QSqlQuery q;
    QString newSalt = generateSalt(), newHash;
    newHash = generatePasswordHash(_newp, newSalt);
    q.prepare("UPDATE users SET (salt, password_hash, mtime) = (?, ?, ?) WHERE username = ?");
    q.addBindValue(newSalt);
    q.addBindValue(newHash);
    q.addBindValue(StringHelper::currentDateTimeString());
    q.addBindValue(uniqueName);
    return q.exec() && q.numRowsAffected();
}

bool UserManager::changeUserName(const QString& oldname, const QString& _newname){
    QSqlQuery q;
    q.prepare("UPDATE users SET username = ? WHERE username = ?");
    q.addBindValue(_newname);
    q.addBindValue(oldname);
    return q.exec() && q.numRowsAffected();
}

bool UserManager::changeUserName(qint64 id, const QString& _newname){
    QSqlQuery q;
    q.prepare("UPDATE users SET username = ? WHERE user_id = ?");
    q.addBindValue(_newname);
    q.addBindValue(id);
    return q.exec() && q.numRowsAffected();
}

bool UserManager::setFullName(qint64 user_id, const QString& fullname) {
    QSqlQuery q;
    q.prepare("UPDATE users SET (full_name, updated_at) = (?, 'CURRENT_TIMESTAMP') WHERE user_id = ?");
    q.addBindValue(fullname);
    q.addBindValue(user_id);
    return q.exec();
}

bool UserManager::setEmail(qint64 user_id, const QString& email) {
    QSqlQuery q;
    q.prepare("UPDATE users SET (email, updated_at) = (?, CURRENT_TIMESTAMP) WHERE user_id = ?");
    q.addBindValue(email);
    q.addBindValue(user_id);
    return q.exec();
}

bool UserManager::setPhone(qint64 user_id, const QString& phone) {
    QSqlQuery q;
    q.prepare("UPDATE users SET (phone_number, updated_at) = (?, CURRENT_TIMESTAMP) WHERE user_id = ?");
    q.addBindValue(phone);
    q.addBindValue(user_id);
    return q.exec();
}

bool UserManager::setAddress(qint64 user_id, const QString& addr) {
    QSqlQuery q;
    q.prepare("UPDATE users SET (home_address, updated_at) = (?, CURRENT_TIMESTAMP) WHERE user_id = ?");
    q.addBindValue(addr);
    q.addBindValue(user_id);
    return q.exec();
}

bool UserManager::setFullName(const QString& username, const QString& fullname) {
    if(getUserId(username) < 1) return false;
    return setFullName(getUserId(username), fullname);
}
bool UserManager::setEmail(const QString& username, const QString& email) {
    if(getUserId(username) < 1) return false;
    return setEmail(getUserId(username), email);
}

bool UserManager::setPhone(const QString& username, const QString& phone) {
    if(getUserId(username) < 1) return false;
    return setPhone(getUserId(username), phone);
}

bool UserManager::setAddress(const QString& username, const QString& addr) {
    if(getUserId(username) < 1) return false;
    return setAddress(getUserId(username), addr);
}

qint64 UserManager::getUserId(const QString& name) {
    QSqlQuery q;
    q.prepare("SELECT user_id FROM users WHERE username = ?");
    q.addBindValue(name);
    if(q.exec() && q.next()) {
        return q.value(0).toLongLong();
    }
    return -1;
};

qint64 UserManager::userCount() const {
    QSqlQuery q("SELECT COUNT(*) FROM users");
    q.next();
    return q.value(0).toLongLong();
}

void UserManager::setLastActive(qint64 id) {
    QSqlQuery q;
    q.prepare("UPDATE users SET last_active = CURRENT_TIMESTAMP WHERE user_id = ?;");
    q.addBindValue(id);
    q.exec();
}

User User::fromRecord(const QSqlRecord& rec) {
    User s;
    s.user_id = rec.value("user_id").toLongLong();
    s.username = rec.value("username").toString();
    s.salt = rec.value("salt").toString();
    s.password_hash = rec.value("password_hash").toString();
    s.full_name = rec.value("full_name").toString();
    s.email = rec.value("email").toString();
    s.phone_number = rec.value("phone_number").toString();
    s.home_address = rec.value("home_address").toString();
    s.is_active = rec.value("is_active").toBool();
    s.created_at = rec.value("created_at").toDateTime();
    s.updated_at = rec.value("updated_at").toDateTime();
    return s;
}
