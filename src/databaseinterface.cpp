#include "databaseinterface.h"
#include <QSqlQuery>
#include <QSqlError>

DatabaseInterface &DatabaseInterface::instance() {
  static DatabaseInterface di;
  return di;
};

bool DatabaseInterface::saveInvoiceData(const InvoiceData &ida){
  auto db = QSqlDatabase::database("JUST-INV_DB", true);
  db.transaction();
  QSqlQuery qi(db);
  qi.prepare( R"--( 
    INSERT INTO invoices ( invoice_date, admin, customer, customer_phone, total, created_by)
    VALUES (:invoice_date, :admin, :customer, :customer_phone, :total, :admin); )--");
  qi.bindValue(":invoice_date", ida.dateString);
  qi.bindValue(":admin", ida.adminName);
  qi.bindValue(":total", ida.total);
  qi.bindValue(":customer", ida.customerName);
  qi.bindValue(":customer_phone", ida.customerPhone);
  if (!qi.exec()) {
    qFatal() << "Tidak dapat menyimpan" << qi.lastError().text();
    db.rollback();
    emit saveDone(false);
    return false;
  }
  int inv_id = qi.lastInsertId().toInt();
  QVariantList idList, productList, priceList, qtyList, ttlList;
  for(const auto &item : ida.itemList) {
    idList << inv_id;
    productList << item.productName;
    priceList << item.unitPrice;
    qtyList << item.unitQty;
    qDebug() << inv_id << item.productName << item.unitPrice << item.unitQty ;
  }
  QSqlQuery qa(db);
  qa.prepare(R"--(
      INSERT INTO invoice_item (invoice_id, order_name, price, qty)
      VALUES (?, ?, ?, ?);
  )--");
  qa.addBindValue(idList);
  qa.addBindValue(productList);
  qa.addBindValue(priceList);
  qa.addBindValue(qtyList);
  if(!qa.execBatch()) {
    qDebug() << "ExecBatch failed" << qa.lastError().text();
    db.rollback();
    emit saveDone(false);
    return false;
  }
  if(!db.commit()) {
    qDebug() << "Commit Failed" << db.lastError().text(); 
  }
  emit saveDone(true);
  return true;
}


// bool DatabaseInterface::saveInvoiceData(const QVariant &va){
  // return saveInvoiceData(va.value<InvoiceData>());
// }
