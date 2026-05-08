#pragma once

#include <QVariantMap>

struct LoginParam {
    QString username;
    QString password;
};

struct UserData
{
    int id;
    int role_id;
    QString nama_lengkap;
    QString email;
    QString nomor_telp;
};

class QSqlRecord;
class UserController
{
public:
    UserController() = default;
    ~UserController() = default;
    bool createUser(QVariantMap &params, QString *error);
    bool updateUser(int userId, const QVariantMap &params, QString *error);
    UserData getUserData(int id) const;
    QSqlRecord getUserRecord(int id) const;
};