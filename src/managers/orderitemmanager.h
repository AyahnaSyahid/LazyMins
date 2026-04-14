#pragma once
#include "basemanager.h"

class OrderItemManager : public BaseManager
{
public:
    explicit OrderItemManager();

    QList<QSqlRecord> getByOrder(int orderId);
    bool updateFinishingTotal(int id, int finishingTotal);

protected:
    // Setelah insert/update item, recalculate subtotal order induk
    bool afterCreate(const QSqlRecord& record) override;
    bool afterUpdate(int id, const QSqlRecord& before, const QSqlRecord& after) override;
    bool afterDelete(int id, const QSqlRecord& before) override;

private:
    bool recalculateOrderSubtotal(int orderId);
};
