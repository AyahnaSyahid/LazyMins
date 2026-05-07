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

QVariantMap AkunTransaksiController::getAccountData(int accId) {
  AkunTransaksiManager manager;
  auto opt = manager.getById(accId);
  if (!opt) return {};
  QVariantMap retVal;
  auto rc = opt.value();
  for(int field = 0; field < opt->count(); ++field) {
    retVal[rc.fieldName(field)] = rc.value(field);
  }
  return retVal;
}

bool AkunTransaksiController::makeOpname(int accId, const QVariantMap &params,
                                         QString *error) {
  AkunTransaksiManager manager;
  if (manager.opname(accId, params["real_saldo"].toInt(), params["notes"].toString()))
    return true;
  if (error)
    *error = manager.errorString();
  return false;
}
