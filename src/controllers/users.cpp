#include "users.h"
#include "src/managers/adminmanager.h"

bool UserController::createUser(QVariantMap &params, QString *error)
{
    AdminManager am;
    if (am.exists(params["username"].toString()))
    {
        if (error)
            *error = "Username already exists";
        return false;
    }

    auto opt = am.create(params);
    if (!opt.has_value())
    {
        if (error)
            *error = am.errorString();
        return false;
    }
    params["id"] = opt->value("id").toInt();
    return true;
}

bool UserController::updateUser(int userId, const QVariantMap &params, QString *error)
{
    AdminManager am;
    if (am.update(userId, params))
        return true;

    if (error)
        *error = am.errorString();
    return false;
}
