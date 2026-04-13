#pragma once

#include "basemanager.h"

// ============================================================================
// OrderItemFinishingManager — tabel: order_item_finishings
// ============================================================================

class OrderItemFinishingManager : public BaseManager
{
public:
    explicit OrderItemFinishingManager()
        : BaseManager("order_item_finishings") {}

    QList<QSqlRecord> getByOrderItem(int orderItemId);
    bool removeByOrderItem(int orderItemId);

protected:
    bool afterCreate(const QSqlRecord&);
    bool afterUpdate(int id, const QSqlRecord& a, const QSqlRecord& b);
    bool afterDelete(int id, const QSqlRecord& a, const QSqlRecord& b);
};
