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

bool OrderItemManager::afterCreate(const QSqlRecord& record)
{
    // StockMovementManager smm;
    // ProductManager pm;
    // auto cUser = SessionManager::instance().currentUser();
    // if (!cUser) {
      // setErrorString("Tidak menemukan users aktif");
      // return false;
    // }
    
    // int product_id = record.value("product_id").toInt();
    // auto opt_pr = pm.getById(product_id);
    // if ( !opt_pr ) {
      // setErrorString("Tidak dapat menemukan Produk dengan ID #" + QString::number(product_id));
      // return false;
    // }
    
    // qreal product_stock = opt_pr->value("stock").toDouble();
    // qreal qty = record.value("quantity").toDouble();
    // if (record.value("use_area").toBool()) {
      // qreal size_width = record.value("size_width").toDouble();
      // qreal size_height = record.value("size_height").toDouble();
      // qty = qCeil((size_width * size_height * qty) * 100.0) / 100.0;
    // }
    // auto opt_mv = smm.recordMovement( record.value("product_id").toInt(),
                                      // "out", -qty, product_stock, qCeil( (product_stock - qty) * 100.0 ) / 100.0,
                                      // cUser->value("id").toInt(), "AUTO" );
    // if (!opt_mv) {
      // setErrorString(smm.errorString());
      // return false;
    // }
    return true;
}

bool OrderItemManager::afterUpdate(int id, const QSqlRecord& before, const QSqlRecord& after)
{
    Q_UNUSED(id)
    Q_UNUSED(before)
    return true;
    // return recalculateOrderSubtotal(after.value("order_id").toInt());
}

bool OrderItemManager::afterDelete(int id, const QSqlRecord& before)
{
    Q_UNUSED(id)
    return true;
    // return recalculateOrderSubtotal(before.value("order_id").toInt());
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