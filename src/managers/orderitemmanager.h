#pragma once

#include "basemanager.h"

// ============================================================================
// OrderItemManager — tabel: order_items
// ============================================================================
class OrderItemManager : public BaseManager
{
public:
    explicit OrderItemManager()
        : BaseManager("order_items") {}

    QList<QSqlRecord> getByOrder(int orderId);
    
    bool removeByOrder(int orderId);
    bool updateItemFinishingTotal(int orderId);

protected:
    bool afterCreate(const QSqlRecord& c) override;
    bool afterUpdate(int, const QSqlRecord&, const QSqlRecord& c) override;
    bool afterDelete(int, const QSqlRecord&) override;
};
