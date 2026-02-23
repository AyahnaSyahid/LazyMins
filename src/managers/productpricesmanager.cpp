#include "productpricesmanager.h"

bool ProductPricesManager::exists(int prid, int lvid) const {
  QSqlQuery q(BaseManager::connection);
  q.prepare("SELECT COUNT(*) FROM product_prices WHERE product_id = :product_id AND price_level_id = :price_level_id");
  q.bindValue(":product_id", prid);
  q.bindValue(":price_level_id", lvid);
  if(q.exec() && q.next()) {
    return q.value(0).toInt() > 0;
  }
  return false;
}

bool ProductPricesManager::setPrice(int pid, int levid, int newprice) {
  QSqlQuery q(BaseManager::connection);
  q.prepare(R"--(
    INSERT INTO product_prices (product_id, price_level_id, price)
    VALUES (:product_id, :price_level_id, :price)
      ON CONFLICT (product_id, price_level_id) 
        DO UPDATE SET (price, updated_at) = (excluded.newprice, CURRENT_TIMESTAMP)
    )--");
  q.bindValue(":product_id", pid);
  q.bindValue(":price_level_id", levid);
  q.bindValue(":price", newprice);
  if(!q.exec()) {
    qDebug() << q.lastError().text();
    return false;
  }
  return true;
}
