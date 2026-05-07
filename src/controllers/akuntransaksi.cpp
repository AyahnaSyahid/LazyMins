#include "akuntransaksi.h"

#include "src/managers/akuntransaksimanager.h"

bool AkunTransaksiController::createAccount(QVariantMap &params, QString* error)
{
    AkunTransaksiManager manager;
    auto opt = manager.create(params);
    if (!opt.has_value())
    {
        if (error)
            *error = manager.errorString();
        return false;
    }
    params["id"] = opt->value("id").toInt();
    return true;
}

bool AkunTransaksiController::updateAccount(int accId, QVariantMap &params, QString *error)
{
    AkunTransaksiManager manager;
    if (manager.update(accId, params))
        return true;
    if (error)
        *error = manager.errorString();
    return false;
}
