#include "orderitemfinishingmanager.h"
#include "managers.h"

// ============================================================================
// OrderItemFinishingManager
// ============================================================================

QList<QSqlRecord> OrderItemFinishingManager::getByOrderItem(int orderItemId)
{
    return getWhere("order_item_id = :order_item_id", {{"order_item_id", orderItemId}});
}

bool OrderItemFinishingManager::removeByOrderItem(int orderItemId)
{
    QSqlQuery q(BaseManager::connection);
    q.prepare(QString("DELETE FROM %1 WHERE order_item_id = :order_item_id").arg(tableName()));
    q.bindValue(":order_item_id", orderItemId);
    return q.exec();
}

bool OrderItemFinishingManager::afterCreate(const QSqlRecord& r) {
  // update harga order terkait finishing ini
  OrderItemManager oim;
  if (!oim.updateItemFinishingTotal(r.value("id").toInt())) {
    QString err = oim.errorString();
    qWarning() << "[OrderItemFinishingManager::afterCreate] Error:"
               << err;
    setErrorString(err);
    return false;
  }
  return true;
}

bool OrderItemFinishingManager::afterUpdate(int id, const QSqlRecord& a, const QSqlRecord& b) {
  qint64 sub_a = a.value("subtotal").toLongLong();
  qint64 sub_b = b.value("subtotal").toLongLong();
  qint64 id_a = b.value("order_item_id").toLongLong();
  qint64 id_b = b.value("order_item_id").toLongLong();
  
  if ((sub_a != sub_b) || (id_a != id_b)) {
    OrderItemManager oim;
    if (!oim.updateItemFinishingTotal(id_a)) {
      QString err = oim.errorString();
      qWarning() << "[OrderItemFinishingManager::afterUpdate] Error:"
                 << err;
      setErrorString(err);
      return false;
    }
    if (id_a != id_b) {
      if (!oim.updateItemFinishingTotal(id_b)) {
        QString err = oim.errorString();
        qWarning() << "[OrderItemFinishingManager::afterUpdate] Error:"
                   << err;
        setErrorString(err);
        return false;
      }
    }
  }
  return true;
}

bool OrderItemFinishingManager::afterDelete(int id, const QSqlRecord& a, const QSqlRecord& b) {
  OrderItemManager oim;
  if (!oim.updateItemFinishingTotal(a.value("order_item_id").toInt())) {
      QString err = oim.errorString();
      qWarning() << "[OrderItemFinishingManager::afterDelete] Error:"
                 << err;
      setErrorString(err);
      return false;
  }
  return true;
}