#include "productmanager.h"

// ============================================================================
// ProductManager
// ============================================================================

QList<QSqlRecord> ProductManager::getActive(const QString& orderBy)
{
    return getWhere("is_active = 1", {}, orderBy);
}

QList<QSqlRecord> ProductManager::getByCategory(int categoryId)
{
    return getWhere("category_id = :category_id AND is_active = 1",
                    {{"category_id", categoryId}}, "name");
}

QList<QSqlRecord> ProductManager::getLowStock()
{
    return getWhere("is_active = 1 AND stock <= min_stock", {}, "name");
}

std::optional<QSqlRecord> ProductManager::findBySku(const QString& sku)
{
    auto rows = getWhere("sku = :sku COLLATE NOCASE", {{"sku", sku}});
    if (!rows.isEmpty()) return rows.first();
    return std::nullopt;
}
std::optional<QSqlRecord> ProductManager::findByName(const QString& name)
{
    auto rows = getWhere("name = :name COLLATE NOCASE", {{"name", name}});
    if (!rows.isEmpty()) return rows.first();
    return std::nullopt;
}

bool ProductManager::adjustStock(int id, qreal delta, const QString& notes)
{
    Q_UNUSED(notes) // caller should record a StockMovement separately
    QSqlQuery q(BaseManager::connection);
    q.prepare(QString("UPDATE %1 SET stock = stock + :delta, updated_at = :updated_at WHERE id = :id")
                  .arg(tableName()));
    q.bindValue(":delta", delta);
    q.bindValue(":updated_at", dateTimeToSql());
    q.bindValue(":id", id);
    if (!q.exec()) {
        qDebug() << "ProductManager::adjustStock error:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}
