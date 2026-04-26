#include "orderitemmanager.h"
#include "ordermanager.h"
#include "stockmovementmanager.h"
#include "productmanager.h"
#include "src/utils/sessionmanager.h"

OrderItemManager::OrderItemManager()
    : BaseManager("order_items", false)
{
}

QList<QSqlRecord> OrderItemManager::getByOrder(int orderId)
{
    return getWhere("order_id = :order_id",
                    {{ "order_id", orderId }},
                    "id");
}

bool OrderItemManager::updateFinishingTotal(int id, int finishingTotal)
{
    return update(id, {{ "finishing_total", finishingTotal }});
}

bool OrderItemManager::recalculate(int item_id)
{
  // mengumpulkan semua harga finishing
  QSqlQuery q(BaseManager::connection);
  
  q.prepare(R"-(
UPDATE order_items
   SET finishing_total = cte.new_value,
       updated_at = CURRENT_TIMESTAMP
  FROM (
           SELECT COALESCE(SUM(subtotal), 0) AS new_value
             FROM order_item_finishings
            WHERE order_item_id = :itid
       )
       AS cte
 WHERE order_items.id = :itid AND
       order_items.finishing_total <> cte.new_value
  )-");
  
  q.bindValue(":itid", item_id);
  if (!q.exec()) {
    setErrorString(q.lastError().text());
    return false;
  }
  return true;
}