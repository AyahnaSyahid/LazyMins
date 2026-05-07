#include "orderitemfinishingmanager.h"

#include "orderitemmanager.h"

OrderItemFinishingManager::OrderItemFinishingManager()
    : BaseManager("order_item_finishings", false) {}

QList<QSqlRecord> OrderItemFinishingManager::getByOrderItem(int orderItemId) {
  return getWhere("order_item_id = :order_item_id",
                  {{"order_item_id", orderItemId}}, "id");
}

void OrderItemFinishingManager::setOrderItem(int id, int orderItemId) {
  update(id, {{"order_item_id", orderItemId}});
}
