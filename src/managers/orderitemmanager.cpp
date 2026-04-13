#include "orderitemmanager.h"
#include "orderitemfinishingmanager.h"

// ============================================================================
// OrderItemManager
// ============================================================================

QList<QSqlRecord> OrderItemManager::getByOrder(int orderId)
{
    return getWhere("order_id = :order_id", {{"order_id", orderId}});
}

bool OrderItemManager::removeByOrder(int orderId)
{
    QSqlQuery q(BaseManager::connection);
    q.prepare(QString("DELETE FROM %1 WHERE order_id = :order_id").arg(tableName()));
    q.bindValue(":order_id", orderId);
    return q.exec();
}

bool OrderItemManager::updateItemFinishingTotal(int orderId) {
  OrderItemFinishingManager oifm;
  
  auto rlist = oifm.getByOrderItem(orderId);
  qint64 sum = 0;
  for (auto const& r : rlist) {
    sum += r.value("subtotal").toLongLong();
  }

  return update(orderId, {{"finishing_total", sum}, {"updated_at", dateTimeToSql()}});
}


bool OrderItemManager::afterCreate(const QSqlRecord& c) {
  return true;
}

bool OrderItemManager::afterUpdate(int, const QSqlRecord&, const QSqlRecord& c) {
  return true;
  
}

bool OrderItemManager::afterDelete(int, const QSqlRecord&) {
  return true;
  
}
