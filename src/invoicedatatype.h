#ifndef INVOICEDATATYPE_H
#define INVOICEDATATYPE_H

#include <QDateTime>

struct InvoiceData {
  struct ItemData {
    QString productName;
    int unitPrice, unitQty, subTotal;
    
    inline bool operator==(const ItemData &other) const {
      return productName == other.productName &&
             unitPrice   == other.unitPrice   &&
             unitQty     == other.unitQty ;
    }
  };

  QString adminName, customerName, customerPhone, dateString;
  int total, paid;

  QList<ItemData> itemList;
  
  bool balance() const { return total - paid; }
  
  inline bool operator==(const InvoiceData &ot) const {
    return total == ot.total &&
           paid  == ot.paid  &&
           adminName  == ot.adminName &&
           customerName  == ot.customerName &&
           customerPhone  == ot.customerPhone &&
           itemList       == ot.itemList;
  }
};

struct PaymentData {
  QString adminName, method;
  int amount, cashback;
  QDateTime paymentTime;
};

struct StoreInfoData {
  QString storeName, storeAddr, storePhone;
};

struct PrintInvoiceParams {
  // store info
  StoreInfoData storeInfo;

  // Invoice Data
  QString invoiceCode, adminName, customerName, customerPhone, invoiceDate;

  // Items Data
  QList<InvoiceData::ItemData> itemList;

  // Payments Data
  QList<PaymentData> paymentList;
};

Q_DECLARE_METATYPE(InvoiceData::ItemData);
Q_DECLARE_METATYPE(InvoiceData);
#endif