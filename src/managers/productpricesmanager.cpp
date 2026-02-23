#include "productpricesmanager.h"

bool ProductPricesManager::exists(int prid, int lvid) const {
  auto found = getWhere("product_id = :prid AND price_level_id = :pli", 
                        {{"prid", prid}, {"pli", lvid}}, 
                        "",
                        1).count();
  return found > 0;
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
