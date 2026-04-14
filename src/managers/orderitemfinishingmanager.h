#pragma once
#include "basemanager.h"

class OrderItemFinishingManager : public BaseManager
{
public:
    explicit OrderItemFinishingManager();

    QList<QSqlRecord> getByOrderItem(int orderItemId);

protected:
    // Setelah insert/update/delete finishing, recalculate finishing_total pada order_item induk
    bool afterCreate(const QSqlRecord& record) override;
    bool afterUpdate(int id, const QSqlRecord& before, const QSqlRecord& after) override;
    bool afterDelete(int id, const QSqlRecord& before) override;

private:
    bool recalculateFinishingTotal(int orderItemId);
};
