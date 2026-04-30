#include "konsumenmanager.h"

KonsumenManager::KonsumenManager()
    : BaseManager("konsumen", false)
{
}

bool KonsumenManager::beforeCreate(QVariantMap &params)
{
    // Auto-generate customer_code jika belum diisi
    if (!params.contains("customer_code") || params["customer_code"].toString().isEmpty())
    {
        params["customer_code"] = generateCode("konsumen", "customer_code", "CUST-", 3);
    }
    return true;
}

std::optional<QSqlRecord> KonsumenManager::getByCode(const QString &customerCode) const
{
    auto results = const_cast<KonsumenManager *>(this)->getWhere(
        "customer_code = :customer_code",
        {{"customer_code", customerCode}});
    if (results.isEmpty())
        return std::nullopt;
    return results.first();
}

QList<QSqlRecord> KonsumenManager::getActive(const QString &orderBy, int limit)
{
    return getWhere("is_active = 1", {}, orderBy, limit);
}

bool KonsumenManager::deactivate(int id)
{
    return update(id, {{"is_active", 0}});
}

bool KonsumenManager::hasAnyRef(int id)
{
    QSqlQuery query(connection);
    query.prepare(R"-(
SELECT EXISTS (
           SELECT 1
             FROM orders
            WHERE customer_id = :csid
       )
OR 
       EXISTS (
           SELECT 1
             FROM invoices
            WHERE customer_id = :csid
       )
       AS ada_referensi;
        )-");
    query.bindValue(":csid", id);
    if (query.exec() && query.next())
    {
        return query.value("ada_referensi").toBool();
    }
    return false;
}
