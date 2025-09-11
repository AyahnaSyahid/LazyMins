#ifndef InvoiceGraphics_H
#define InvoiceGraphics_H

#include <QFont>
#include <QPointF>
#include <QRectF>

struct InvoiceGraphics {
  static QString companyInfo[4];  // Nama, Alamat1, Alamat2, CS_Number
  static QString clossingStatements[4];
  
  QRectF rLogo, rCompanySign, rBarcodeSpace, rCustomerInfo, rTableHeader,
      rOrderItem;

  int imgWidth, ordersCount, totalItemsPrice, itemsDiscount, invoiceDiscount,
      paidValue;

  QString invoiceCode, userName, customerInfo[2], footerSignature[3];

  InvoiceGraphics(const QString &invCode, const QString &userName = "Admin",
                  const QString &cs1 = "Guest", const QString &cs2 = "Unknown");
  
  // sets clossing statement separated by newlines up to 4 lines
  static void setClossingStatements(const QString& s);
  
  // sets clossing statement in line-th become s
  static void setClossingStatement(int line, const QString& s);
};

#endif // InvoiceGraphics_H