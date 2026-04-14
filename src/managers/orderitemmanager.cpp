#include "orderitemmanager.h"
#include "ordermanager.h"

OrderItemManager::OrderItemManager()
    : BaseManager("order_items", false)
{
}

QList<QSqlRecord> OrderItemManager::getByOrder(int orderId)
{
    return getWhere("order_id = :order_id",
                    {{ ":order_id", orderId }},
                    "id");
}

bool OrderItemManager::updateFinishingTotal(int id, int finishingTotal)
{
    return update(id, {{ "finishing_total", finishingTotal }});
}

bool OrderItemManager::afterCreate(const QSqlRecord& record)
{
    return recalculateOrderSubtotal(record.value("order_id").toInt());
}

bool OrderItemManager::afterUpdate(int id, const QSqlRecord& before, const QSqlRecord& after)
{
    Q_UNUSED(id)
    Q_UNUSED(before)
    return recalculateOrderSubtotal(after.value("order_id").toInt());
}

bool OrderItemManager::afterDelete(int id, const QSqlRecord& before)
{
    Q_UNUSED(id)
    return recalculateOrderSubtotal(before.value("order_id").toInt());
}

bool OrderItemManager::recalculateOrderSubtotal(int orderId)
{
    if (orderId <= 0) return true;

    // Hitung ulang subtotal dari semua items (kolom total adalah VIRTUAL di DB)
    QSqlQuery q = baseQuery();
    q.prepare(
        "SELECT COALESCE(SUM(total), 0) AS total_sum "
        "FROM order_items WHERE order_id = :order_id"
    );
    q.bindValue(":order_id", orderId);
    if (!q.exec() || !q.next()) {
        setErrorString(q.lastError().text());
        return false;
    }

    int newSubtotal = q.value("total_sum").toInt();
    OrderManager om;
    return om.updateSubtotal(orderId, newSubtotal);
}
