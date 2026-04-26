#include "finishingservicemanager.h"

FinishingServiceManager::FinishingServiceManager()
    : BaseManager("finishing_services", false)
{
}

bool FinishingServiceManager::beforeCreate(QVariantMap& params)
{
    if (!params.contains("code") || params["code"].toString().isEmpty()) {
        params["code"] = generateCode("finishing_services", "code", "FIN-", 3);
    }
    return true;
}

std::optional<QSqlRecord> FinishingServiceManager::getByCode(const QString& code) const
{
    auto results = const_cast<FinishingServiceManager*>(this)->getWhere(
        "code = :code COLLATE NOCASE",
        {{ "code", code }}
    );
    if (results.isEmpty()) return std::nullopt;
    return results.first();
}

QList<QSqlRecord> FinishingServiceManager::getActive(const QString& orderBy, int limit)
{
    return getWhere("is_active = 1", {}, orderBy, limit);
}

bool FinishingServiceManager::deactivate(int id)
{
    return update(id, {{ "is_active", 0 }});
}
