#include "konsumenmanager.h"

KonsumenManager::KonsumenManager()
    : BaseManager("konsumen", false)
{
}

bool KonsumenManager::beforeCreate(QVariantMap& params)
{
    // Auto-generate customer_code jika belum diisi
    if (!params.contains("customer_code") || params["customer_code"].toString().isEmpty()) {
        params["customer_code"] = generateCode("konsumen", "customer_code", "CUST-", 3);
    }
    return true;
}

std::optional<QSqlRecord> KonsumenManager::getByCode(const QString& customerCode) const
{
    auto results = const_cast<KonsumenManager*>(this)->getWhere(
        "customer_code = :customer_code",
        {{ ":customer_code", customerCode }}
    );
    if (results.isEmpty()) return std::nullopt;
    return results.first();
}

QList<QSqlRecord> KonsumenManager::getActive(const QString& orderBy, int limit)
{
    return getWhere("is_active = 1", {}, orderBy, limit);
}

bool KonsumenManager::deactivate(int id)
{
    return update(id, {{ "is_active", 0 }});
}
