#ifndef ProductItemH
#define ProductItemH

#include "databaseitem.h"
#include <QDateTime>

class ProductItem : public DatabaseItem
{
  public:
    qint64 product_id, 
           cost, 
           price;
    QString name,
            code,
            qr_bar,
            description;
    bool use_area;
    QDateTime created_at,
              updated_at;
    ProductItem(qint64 = -1);
    ProductItem(const QString& name);
    
    implementDatabaseItem;
    tableNameAssign(products);
    bool operator==(const DatabaseItem& o) const;
};
#endif // ProductItemH