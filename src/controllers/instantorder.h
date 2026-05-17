#pragma once

#include <QString>
#include <QVariantMap>

class OrderHeader;
class OrderModel;
class InstantOrderController
{
public:
    InstantOrderController() = default;
    ~InstantOrderController() = default;

    bool create(const OrderHeader& oh, OrderModel* omodel, const QVariantMap& paymentAndInvoiceParam, QString* err = nullptr);

private:
    bool commitModel(OrderModel* omodel, QString *err = nullptr) const;
};