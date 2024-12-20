#include "useritem.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QVariant>

// #include <QSqlError>
// #include <QtDebug>

QString generateSalt(int length=10) {
    // Generate a random salt
    QByteArray salt = QByteArray(length, '\0');
    for (int i = 0; i < length; ++i) {
        salt[i] = static_cast<char>(QRandomGenerator::global()->generate() % 256);
    }
    return salt.toBase64(); // Encode salt to Base64 for easy storage
}

QString generatePasswordHash(const QString &password, const QString &salt, unsigned int iterations=10000) {
    QByteArray hash = password.toUtf8();
    QByteArray basalt = QByteArray::fromBase64(salt.toUtf8());
    hash += basalt;
    for (int i = 0; i < iterations; ++i) {
        hash = QCryptographicHash::hash(hash + basalt, QCryptographicHash::Sha256);
    }
    return hash.toHex(); // Return hex representation of the final hash
}

UserItem::UserItem(qint64 id) :
    user_id(id),
    DatabaseItem()
{
    if(id > 0) {
        q.prepare("SELECT * FROM users WHERE user_id = ?");
        q.addBindValue(user_id);
        if(q.exec() && q.next()) {
            // qValueAssign(user_id, qint64)
            qValueAssign(username, QString)
            qValueAssign(salt, QString)
            qValueAssign(password_hash, QString)
            qValueAssign(full_name, QString)
            qValueAssign(home_address, QString)
            qValueAssign(email, QString)
            qValueAssign(phone_number, QString)
            qValueAssign(is_active, bool)
            qValueAssign(created_at, QDateTime)
            qValueAssign(updated_at, QDateTime)
        } else {
            user_id = -1;
        }
    }
    password = "";
}

UserItem::UserItem(const QString& name)
{
    username = name;
    q.prepare("SELECT * FROM users WHERE username = ?");
    q.addBindValue(name);
    if(q.exec() && q.next()) {
        qValueAssign(user_id, qint64)
        // qValueAssign(username, QString)
        qValueAssign(salt, QString)
        qValueAssign(password_hash, QString)
        qValueAssign(full_name, QString)
        qValueAssign(home_address, QString)
        qValueAssign(email, QString)
        qValueAssign(phone_number, QString)
        qValueAssign(is_active, bool)
        qValueAssign(created_at, QDateTime)
        qValueAssign(updated_at, QDateTime)
    } else {
        user_id = -1;
    }
    password = "";
}

bool UserItem::passwordMatch(const UserItem& user, const QString& pass) {
    if(user.password_hash.isEmpty() || user.salt.isEmpty()) return false;
    return user.password_hash == generatePasswordHash(pass, user.salt);
}

bool UserItem::save() {
    if(user_id < 1) {
        // create new
        if(username.isEmpty() && password.isEmpty()) {
            *this = UserItem(-1);
            return false;
        }
        auto nsalt = generateSalt();
        auto npassword_hash = generatePasswordHash(password, nsalt);
        q.prepare(R"_(
            INSERT INTO users ( 
                username, salt, password_hash, full_name,
                home_address, email, phone_number, is_active,
                created_at, updated_at )
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, 
                CURRENT_TIMESTAMP, 
                CURRENT_TIMESTAMP );
                            )_");
        q.addBindValue(username);
        q.addBindValue(nsalt);
        q.addBindValue(npassword_hash);
        q.addBindValue(full_name);
        q.addBindValue(home_address);
        q.addBindValue(email);
        q.addBindValue(phone_number);
        q.addBindValue(is_active);
        if(q.exec()) {
            user_id = q.lastInsertId().toLongLong();
            q.clear();
            *this = UserItem(user_id);
            return true;
        }
    } else {
        // update
        if(username.isEmpty() || salt.isEmpty() || password_hash.isEmpty()) return false;
        QString nsalt = salt, npassword_hash = password_hash;
        if(!password.isEmpty()) {
            nsalt = generateSalt();
            npassword_hash = generatePasswordHash(password, nsalt);
            password = "";
        }
        
        q.prepare(R"_(
            UPDATE users SET (username, salt, password_hash, full_name, home_address, email, phone_number, is_active, updated_at)
                = (?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP) WHERE user_id = ?;
                    )_");
        q.addBindValue(username);
        q.addBindValue(nsalt);
        q.addBindValue(npassword_hash);
        q.addBindValue(full_name);
        q.addBindValue(home_address);
        q.addBindValue(email);
        q.addBindValue(phone_number);
        q.addBindValue(is_active);
        q.addBindValue(user_id);
        if(q.exec()) {
            if(q.numRowsAffected() > 0) {
                q.clear();
                *this = UserItem(user_id);
                return true;
            }
        }
        // if(q.lastError().isValid()) {
            // qDebug() << q.lastError().text();
        // }
    }
    return false;
}

bool UserItem::erase() {
    if(user_id > 0) {
        q.prepare("DELETE FROM users WHERE user_id = ?");
        q.addBindValue(user_id);
        if(q.exec() && q.numRowsAffected() > 0) {
            user_id = -1;
            q.finish();
            return true;
        } else {
            return false;
        }
    }
    return true;
}

bool UserItem::operator==(const DatabaseItem& ot) const {
    auto ptr = dynamic_cast<const UserItem*>(&ot);
    if(!ptr) return false;
    return user_id == ptr->user_id &&
           username == ptr->username &&
           salt == ptr->salt &&
           password_hash == ptr->password_hash &&
           full_name == ptr->full_name &&
           home_address == ptr->home_address &&
           email == ptr->email &&
           phone_number == ptr->phone_number &&
           is_active == ptr->is_active &&
           created_at == ptr->created_at &&
           updated_at == ptr->updated_at &&
           password == ptr->password;
}