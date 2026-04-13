#include "stockmovementmanager.h"

// ============================================================================
// StockMovementManager
// ============================================================================

QList<QSqlRecord> StockMovementManager::getByProduct(int productId, int limit)
{
    return getWhere("product_id = :product_id",
                    {{"product_id", productId}},
                    "movement_date DESC", limit);
}

QList<QSqlRecord> StockMovementManager::getByType(const QString& movementType)
{
    return getWhere("movement_type = :movement_type",
                    {{"movement_type", movementType}}, "movement_date DESC");
}

QList<QSqlRecord> StockMovementManager::getByDateRange(const QDate& from, const QDate& to)
{
    return getWhere(
        "DATE(movement_date) BETWEEN :from AND :to",
        {{"from", dateToSql(from)}, {"to", dateToSql(to)}},
        "movement_date DESC");
}

QList<QSqlRecord> StockMovementManager::getByReference(const QString& referenceType, int referenceId)
{
    return getWhere(
        "reference_type = :rt AND reference_id = :rid",
        {{"rt", referenceType}, {"rid", referenceId}});
}

std::optional<QSqlRecord> StockMovementManager::recordMovement(
    int productId, const QString& type, qreal quantity,
    qreal stockBefore, qreal stockAfter, int adminId,
    const QString& referenceType, int referenceId, const QString& notes)
{
    QVariantMap p;
    p["product_id"]     = productId;
    p["movement_type"]  = type;
    p["quantity"]       = quantity;
    p["stock_before"]   = stockBefore;
    p["stock_after"]    = stockAfter;
    p["admin_id"]       = adminId;
    p["movement_date"]  = dateTimeToSql();
    if (!referenceType.isEmpty()) p["reference_type"] = referenceType;
    if (referenceId > 0)          p["reference_id"]   = referenceId;
    if (!notes.isEmpty())         p["notes"]          = notes;
    return create(p);
}
