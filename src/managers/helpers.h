#pragma once

#include "src/models/ordermodel.h"
#include "src/managers/managers.h"

namespace DBOperationHelper {
  struct OperationResult {
    bool ok; 
    QString error; 
  };
  // digunakan di InstantOrderDialog
  // menggunakan transaction
  OperationResult createInstantOrder( const OrderHeader&, 
                                      const QList<OrderItem> &items, 
                                      const QString& invoiceCode,
                                      const QVariantMap& paymentInfo );
  
  // tanpa transaction
  OperationResult stockUpdate( const OrderItem& it, 
                               const QString& tipe, 
                               const QString& notes,
                               int   adminId  = 1 );
  
  // digunakan di StockOpnameDialog
  // menggunakan transaction
  OperationResult adjustProductStock( int product_id, qreal _final, const QString& notes);
  
  // digunakan di StockRefillDialog
  // menggunakan transaction
  
  OperationResult refillProductStock ( int productId, qreal stockIn, const QString& supplier, const QString& notes);
}