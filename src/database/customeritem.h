#ifndef CustomerItemH
#define CustomerItemH

#include "databaseitem.h"
#include "useritem.h"
#include <QDateTime>

class CustomerItem : public DatabaseItem
{
  public:
    qint64 customer_id;
    QString customer_name,
            email,
            phone_number,
            address;
    UserItem created_by;
    QDateTime created_at;
    
    CustomerItem(qint64 cid=-1);
    CustomerItem(const QString& name);

    implementDatabaseItem;
    tableNameAssign(customers)
    
    bool operator==(const DatabaseItem& di) const override;
};

#endif // CustomerItem