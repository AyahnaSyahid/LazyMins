#pragma once

#include "src/models/ordermodel.h"
#include "src/managers/managers.h"

namespace DBOperationHelper {
  struct CreateInstantOrderResult {
    bool ok; 
    QString error; 
  };
  CreateInstantOrderResult createInstantOrder( const OrderHeader&, 
                                               const QList<OrderItem> &items, 
                                               const QString& invoiceCode,
                                               const QVariantMap& paymentInfo);
}