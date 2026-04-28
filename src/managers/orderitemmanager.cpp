#include "orderitemmanager.h"
#include "orderitemfinishingmanager.h"
#include "productmanager.h"
#include "src/utils/sessionmanager.h"
#include "stockmovementmanager.h"

OrderItemManager::OrderItemManager() : BaseManager("order_items", false) {}

QList<QSqlRecord> OrderItemManager::getByOrder(int orderId)
{
  return getWhere("order_id = :order_id", {{"order_id", orderId}}, "id");
}

bool OrderItemManager::updateFinishingTotal(int id)
{
  QSqlQuery q(BaseManager::connection);

  // COALESCE memastikan jika hasil SUM adalah NULL, maka akan diubah menjadi 0
  q.prepare(
      "UPDATE order_items "
      "SET finishing_total = COALESCE("
      "  (SELECT SUM(subtotal) "
      "   FROM order_item_finishings "
      "   WHERE order_item_id = :id1), "
      "  0) "
      "WHERE id = :id2");

  // Kita bind ID dua kali karena muncul di subquery dan clause WHERE
  q.bindValue(":id1", id);
  q.bindValue(":id2", id);

  if (!q.exec())
  {
    qDebug() << "Update Error:" << q.lastError().text();
    return false;
  }

  return true;
}

bool OrderItemManager::addFinishings(int id, QList<int> finishingIds)
{
  OrderItemFinishingManager oifm;
  for (int finishingId : finishingIds)
  {
    oifm.setOrderItem(finishingId, id);
  }
  return updateFinishingTotal(id);
}

bool OrderItemManager::setOrderId(int id, int orderId)
{
  return update(id, {{"order_id", orderId}});
}

bool OrderItemManager::removeFinishings(int id, QList<int> finishingIds)
{
  OrderItemFinishingManager oifm;
  for (int finishingId : finishingIds)
  {
    oifm.remove(finishingId);
  }
  return updateFinishingTotal(id);
}

bool OrderItemManager::beforeCreate(QVariantMap &params)
{
  if (params.contains("use_area") && params["use_area"].toInt() == 0)
  {
    params["size_width"] = 1;
    params["size_height"] = 1;
  }
  return true;
}

bool OrderItemManager::beforeUpdate(int id, QVariantMap &params)
{
  if (params.contains("use_area") && params["use_area"].toInt() == 0)
  {
    params["size_width"] = 1;
    params["size_height"] = 1;
  }
  return true;
}
