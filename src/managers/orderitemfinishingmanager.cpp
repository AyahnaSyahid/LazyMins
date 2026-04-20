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
  return true;
  // return recalculateFinishingTotal(record.value("order_item_id").toInt());
}

bool OrderItemFinishingManager::afterUpdate(int id, const QSqlRecord& before, const QSqlRecord& after)
{
    Q_UNUSED(id)
    Q_UNUSED(before)
    return true;
    // return recalculateFinishingTotal(after.value("order_item_id").toInt());
}

bool OrderItemFinishingManager::afterDelete(int id, const QSqlRecord& before)
{
    Q_UNUSED(id)
    return true;
    // return recalculateFinishingTotal(before.value("order_item_id").toInt());
}