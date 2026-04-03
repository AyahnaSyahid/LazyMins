#pragma once

#include "src/models/ordermodel.h"
#include "src/managers/managers.h"

namespace DBOperationHelper {
  struct OperationResult {
    bool ok; 
    QString error; 
  };
  OperationResult createInstantOrder( const OrderHeader&, 
                                      const QList<OrderItem> &items, 
                                      const QString& invoiceCode,
                                      const QVariantMap& paymentInfo );
  OperationResult stockUpdate( const OrderItem& it, 
                               const QString& tipe, 
                               const QString& notes,
                               int   adminId  = 1 );
}