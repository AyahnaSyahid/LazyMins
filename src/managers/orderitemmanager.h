#pragma once
#include "basemanager.h"

class OrderItemManager : public BaseManager
{
public:
    explicit OrderItemManager();

    QList<QSqlRecord> getByOrder(int orderId);
    bool updateFinishingTotal(int id);
    bool addFinishings(int id, QList<int> finishingIds);
    bool setOrderId(int id, int orderId);
    bool removeFinishings(int id, QList<int> finishingIds);

protected:
    bool beforeCreate(QVariantMap& params) override;
    bool beforeUpdate(int id, QVariantMap& params) override;
};
