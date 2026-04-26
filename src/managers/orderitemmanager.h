#pragma once
#include "basemanager.h"

class OrderItemManager : public BaseManager
{
public:
    explicit OrderItemManager();

    QList<QSqlRecord> getByOrder(int orderId);
    bool updateFinishingTotal(int id, int finishingTotal);
    bool recalculate(int item_id);

};
