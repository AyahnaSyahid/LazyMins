#include "invoicegraphics.h"

QString InvoiceGraphics::companyInfo[4] {
  "Aksarajaya Group", 
  "Jl. Kapten Naseh 40", 
  "Cipedes, Tasikmalaya", 
  "WA: 12-345-678-1"};

QString InvoiceGraphics::clossingStatements[4] {
  "Terimakasih masih menjadi pelanggan setia kami",
  "Have a nice day !",
  "Kritik & Saran",
  "WA: 12-345-678-1"};

InvoiceGraphics::InvoiceGraphics(
  const QString &invCode,
  const QString &usrName,
  const QString &cust1,
  const QString &cust2):
    imgWidth(945),
    invoiceCode(invCode),
    userName(usrName),
    ordersCount(1),
    totalItemsPrice(0),
    itemsDiscount(0),
    invoiceDiscount(0),
    paidValue(0)
{
  float step = 20.0f;
  QRectF baseRect(40.0f, 0.0f, 865.0f, 231.0f);
  rLogo = baseRect.adjusted(0, 20, -baseRect.width() / 2.0f, -20);
  rCompanySign = baseRect.adjusted(baseRect.width() / 2.0f, 0, 0, 0);
  rBarcodeSpace = baseRect.adjusted(-40.0f, 0, 40.0f, 0.0f).translated(0, baseRect.height() + step);
  rBarcodeSpace.setHeight(120.0f);
  rCustomerInfo = rBarcodeSpace.translated(0.0f, rBarcodeSpace.height() + step).adjusted(40.0f, 0.0f, -40.0f, -50.0f);
  rCustomerInfo.setHeight(120.0f);
  rTableHeader = rCustomerInfo.translated(0.0f, rCustomerInfo.height() + step).adjusted(0.0f, 0.0f, 0.0f, 0.0f);
  rOrderItem = rTableHeader.translated(0.0f, 100.0f);
  rOrderItem.setHeight(114.0f);
}

void InvoiceGraphics::setClossingStatements(const QString& withNewLine) {
  QStringList split = withNewLine.split("\n");
  // InvoiceGraphics::clossingStatements;
  for(int i=0; i<4; ++i) {
    InvoiceGraphics::clossingStatements[i] = split.at(i);
  }
}

void InvoiceGraphics::setClossingStatement(int nth, const QString& statement) {
  if(nth > 0 && nth < 4)
    InvoiceGraphics::clossingStatements[nth] = statement;
}