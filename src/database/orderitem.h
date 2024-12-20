#ifndef OrderItemH
#define OrderItemH

#include "databaseitem.h"
#include <QDateTime>
class QSqlRecord;
class OrderItem : public DatabaseItem {
  public:
    OrderItem(qint64 oid=-1);
    ~OrderItem();
    implementDatabaseItem;
    tableNameAssign(orders);
    
    qint64 order_id = 0,
           product_id = 0,
           customer_id = 0,
           invoice_id = 0,
           quantity = 1, 
           cost_production = 0, 
           selling_price = 0, 
           discount_amount = 0,
           width_mm = 1000,
           height_mm = 1000,
           modified_by,
           total_price;

    // qreal  discount_percentage;
    QString order_name;
    QDateTime order_date,
              modified_date;

  private:
    OrderItem fromRecord(const QSqlRecord&);
};

#endif // OrderItemH