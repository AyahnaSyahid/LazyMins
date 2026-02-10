#ifndef INVOICEDATATYPE_H
#define INVOICEDATATYPE_H

#include <QDateTime>

struct InvoiceData {
  QString adminName, 
          customerName, 
          customerPhone, 
          dateString;
  
  int total;
  
  struct ItemData {
    QString productName;
  
    int unitPrice, 
        unitQty, 
        subTotal;
  };
  
  QList<ItemData> itemList;
};

struct PrintInvoiceParams {
  QString storeName,
          storeAddr,
          storePhone,
          invoiceCode;
  QDateTime printTime;
  QString adminName,
          customerName;
  
};

#endif