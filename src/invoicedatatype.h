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

struct PaymentData {
  QString adminName,
          method;
  int amount;
  QDateTime paymentTime;
};

struct StoreInfoData {
  QString storeName,
          storeAddr,
          storePhone;
};

struct PrintInvoiceParams {
  // store info
  StoreInfoData storeInfo;
  
  // Invoice Data
  QString invoiceCode,
          adminName,
          customerName,
          invoiceDate;

  // Items Data
  QList<InvoiceData::ItemData> itemList;
  
  // Payments Data
  QList<PaymentData> paymentList;
};

Q_DECLARE_METATYPE(InvoiceData::ItemData);
Q_DECLARE_METATYPE(InvoiceData);
#endif