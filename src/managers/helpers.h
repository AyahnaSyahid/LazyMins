#pragma once

#include "src/models/ordermodel.h"

struct Receipt;
namespace DBOperationHelper {
  struct OperationResult {
    bool ok; 
    QString error;
    
    // neded if caller requires some value after database item creation
    QVariantMap data; // maybe empty
  };

  // digunakan di InstantOrderDialog
  // menggunakan transaction
  OperationResult createInstantOrder( const OrderHeader&, 
                                      const QList<OrderItem> &items, 
                                      const QString& invoiceCode,
                                      const QVariantMap& paymentInfo );
  
  // tanpa transaction
  OperationResult internalCreateInvoice( const QVariantMap& param, QList<int> oids);
  
  // menggunakan transaction
  OperationResult createInvoiceForOrders(const QVariantMap& iPar,  QList<int> orderIds);
  OperationResult createPaymentForOrders(const QVariantMap& iPar,  const QVariantMap& pPar, QList<int> orderIds);

  // digunakan di StockOpnameDialog
  // menggunakan transaction
  OperationResult adjustProductStock( int product_id, qreal _final, const QString& notes);

  // digunakan di StockRefillDialog
  // menggunakan transaction  
  OperationResult refillProductStock ( int productId, qreal stockIn, const QString& supplier, const QString& notes);
  
  int currentAdminId();
  
  // menggunakan transaction  
  OperationResult loadInvoiceData(int invoice_id, Receipt *rec);
  OperationResult loadInvoiceDataFast(int invoice_id, Receipt *rec);

  OperationResult paymentHasCompletePaidInvoice(int payment_id);

}