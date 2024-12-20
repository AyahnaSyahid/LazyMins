#ifndef UserManagerH
#define UserManagerH

#include <QString>
#include <QSqlRecord>
#include <QDateTime>

struct User {
    User() : user_id(-1) {};
    static User fromRecord(const QSqlRecord& rc);
    
    qint64 user_id;
    QString username, salt, password_hash,
            full_name, email, phone_number,
            home_address;
    bool is_active;
    QDateTime created_at, updated_at;
};

class UserManager
{
  public:
    QString generateSalt(int l=10) const;
    QString generatePasswordHash(const QString& pass, const QString& salt, unsigned int iteration_count=10000) const;
    bool createUser(const QString& username, const QString& password, qint64* createdId = nullptr);
    bool removeUser(const QString& username);
    bool userExists(const QString& username) const;
    bool userExists(qint64 user_id) const;
    bool passwordMatch(qint64 id, const QString& password) const;
    bool passwordMatch(const QString& username, const QString& password) const;
    bool changeUserPassword(const QString& userName, const QString& password_new);
    bool changeUserPassword(qint64 user_id, const QString& password_new);
    bool changeUserName(const QString& username_old, const QString& username_new);
    bool changeUserName(qint64 user_id, const QString& username_new);
    bool setFullName(qint64 user_id, const QString& fullname);
    bool setEmail(qint64 user_id, const QString& email);
    bool setPhone(qint64 user_id, const QString& phone);
    bool setAddress(qint64 user_id, const QString& addr);
    bool setFullName(const QString& username, const QString& fullname);
    bool setEmail(const QString& username, const QString& email);
    bool setPhone(const QString& username, const QString& phone);
    bool setAddress(const QString& username, const QString& addr);
    void setLastActive(qint64 id);
    qint64 getUserId(const QString& name);
    qint64 userCount() const;
    bool hasPermission(qint64 user_id, const QString& perm);
};

#endif // UserManagerH