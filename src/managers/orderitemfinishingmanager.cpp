#include "orderitemfinishingmanager.h"
#include "orderitemmanager.h"

OrderItemFinishingManager::OrderItemFinishingManager()
    : BaseManager("order_item_finishings", false)
{
}

QList<QSqlRecord> OrderItemFinishingManager::getByOrderItem(int orderItemId)
{
    return getWhere("order_item_id = :order_item_id",
                    {{ ":order_item_id", orderItemId }},
                    "id");
}

bool OrderItemFinishingManager::afterCreate(const QSqlRecord& record)
{
    return recalculateFinishingTotal(record.value("order_item_id").toInt());
}

bool OrderItemFinishingManager::afterUpdate(int id, const QSqlRecord& before, const QSqlRecord& after)
{
    Q_UNUSED(id)
    Q_UNUSED(before)
    return recalculateFinishingTotal(after.value("order_item_id").toInt());
}

bool OrderItemFinishingManager::afterDelete(int id, const QSqlRecord& before)
{
    Q_UNUSED(id)
    return recalculateFinishingTotal(before.value("order_item_id").toInt());
}

bool OrderItemFinishingManager::recalculateFinishingTotal(int orderItemId)
{
    if (orderItemId <= 0) return true;

    // subtotal kolom pada order_item_finishings adalah VIRTUAL (quantity * finishing_price)
    // kita SUM dari baris yang ada
    QSqlQuery q = baseQuery();
    q.prepare(
        "SELECT COALESCE(SUM(subtotal), 0) AS finishing_sum "
        "FROM order_item_finishings WHERE order_item_id = :order_item_id"
    );
    q.bindValue(":order_item_id", orderItemId);
    if (!q.exec() || !q.next()) {
        setErrorString(q.lastError().text());
        return false;
    }

    int newFinishingTotal = q.value("finishing_sum").toInt();
    OrderItemManager oim;
    return oim.updateFinishingTotal(orderItemId, newFinishingTotal);
}
