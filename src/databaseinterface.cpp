#include "databaseinterface.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>

namespace {
  QSqlRecord createInvoice(const InvoiceData& ida, QSqlDatabase &db) {
    QSqlQuery qi(db);
    qi.prepare( R"--( 
      INSERT INTO invoices ( invoice_date, admin, customer, customer_phone, total, created_by)
      VALUES (:invoice_date, :admin, :customer, :customer_phone, :total, :admin); )--");
    qi.bindValue(":invoice_date", ida.dateString);
    qi.bindValue(":admin", ida.adminName);
    qi.bindValue(":customer", ida.customerName);
    qi.bindValue(":customer_phone", ida.customerPhone.isEmpty() ? QVariant(QMetaType::fromType<QString>()): ida.customerPhone);
    qi.bindValue(":total", ida.total);
    if (qi.exec()) {
      int invId = qi.lastInsertId().toInt();
      qi.exec(QString("SELECT * FROM invoices WHERE id = %1").arg(invId)) && qi.next();
      return qi.record();
    }
    qDebug() << "createInvoice" << qi.lastError().text();
    return QSqlRecord();
  }
  
  bool appendInvoiceItems(const QSqlRecord inv, const QList<InvoiceData::ItemData>& dataList, QSqlDatabase &db) {
    QVariantList idList, productList, priceList, qtyList;
    int inv_id = inv.value("id").toInt();
    for(const auto &item : dataList) {
      idList << inv_id;
      productList << item.productName;
      priceList << item.unitPrice;
      qtyList << item.unitQty;
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
      qDebug() << "appendInvoiceItems" << qa.lastError().text();
      return false;
    }
    return true;
  }
  
  QSqlRecord createPayment(const QSqlRecord& inv, int amount, const QString& method, QSqlDatabase  db) {
    QSqlQuery q(db);
    q.prepare(R"--(
      INSERT INTO payments (invoice_id, admin, amount, method, payment_time, received_by)
      VALUES (:invoice_id, :admin, :amount, :method, CURRENT_TIMESTAMP, :admin); 
      )--");
    q.bindValue(":invoice_id", inv.value("id"));
    q.bindValue(":admin", inv.value("admin"));
    q.bindValue(":amount", amount);
    q.bindValue(":method", method);
    if ( q.exec() ) {
      int insertId = q.lastInsertId().toInt();
      q.prepare("SELECT * FROM payments WHERE id = :id");
      q.bindValue(":id", insertId);
      if (q.exec() && q.next()) {
        return q.record();
      }
    }
    return QSqlRecord();
  };

}; // namespace END

DatabaseInterface &DatabaseInterface::instance() {
  static DatabaseInterface di;
  return di;
};

bool DatabaseInterface::saveInvoiceData(const InvoiceData &ida){
  auto db = QSqlDatabase::database("JUST-INV_DB", true);
  db.transaction();
  QSqlRecord inv_record = createInvoice(ida, db);
  if (inv_record.isEmpty()) {
    db.rollback();
    emit saveDone(false);
    return false;
  }
  if(!appendInvoiceItems(inv_record, ida.itemList, db)) {
    db.rollback();
    emit saveDone(false);
    return false;
  }
  if(!db.commit()) {
    db.rollback();
    emit saveDone(false);
    return false;
  }
  emit saveDone(true);
  emit tableUpdate( {"invoices", "invoice_item"});
  return true;
}

StoreInfoData DatabaseInterface::getStoreInfo(QSqlDatabase &db) const {
  StoreInfoData storeInfo {};
  QSqlQuery q(db);
  
  if (!q.exec("SELECT key, val FROM store_data WHERE key IN ('storeName', 'storeAddr', 'storePhone')")) {
    qWarning() << "Query failed:" << q.lastError().text();
    return storeInfo;
  }
  
  while (q.next()) {
    const QString key = q.value(0).toString();
    const QString val = q.value(1).toString();
    
    if (key == "storeName") storeInfo.storeName = val;
    else if (key == "storeAddr") storeInfo.storeAddr = val;
    else if (key == "storePhone") storeInfo.storePhone = val;
  }
  
  return storeInfo;
}

PrintInvoiceParams DatabaseInterface::getPrintInvoiceParams(int invoice_id) const {
  PrintInvoiceParams params {};
  auto db = QSqlDatabase::database("JUST-INV_DB", true);
  db.transaction();
  params.storeInfo = getStoreInfo(db);
  
  QSqlQuery q(db);
  q.prepare("SELECT * FROM invoices WHERE id = :id");
  q.bindValue(":id", invoice_id);
  QSqlRecord invRec;
  if (q.exec() && q.next()) {
    invRec = q.record();
  } else {
    db.rollback();
    return PrintInvoiceParams {};
  }
  
  params.invoiceCode = invRec.value("code").isNull() ? 
      QString("INV-%1-%2")
        .arg(invRec.value("invoice_date").toString().remove('-'))
        .arg(invRec.value("id").toString(), 4, QChar('0')) :
          invRec.value("code").toString();
  params.adminName = invRec.value("admin").toString();
  params.customerName = invRec.value("customer").toString();
  params.invoiceDate = invRec.value("invoice_date").toString();
  
  q.prepare("SELECT * FROM invoice_item WHERE invoice_id = :invoice_id");
  q.bindValue(":invoice_id", invoice_id);
  
  if (!q.exec()) {
    db.rollback();
    return PrintInvoiceParams {};
  }
  
  while (q.next()) {
    InvoiceData::ItemData itd;
    itd.productName = q.value("order_name").toString(); 
    itd.unitPrice = q.value("price").toInt();
    itd.unitQty = q.value("qty").toInt();
    itd.subTotal = q.value("subTotal").toInt();
    params.itemList << itd;
  }
  
  q.prepare("SELECT * FROM payments WHERE invoice_id = :invoice_id");
  q.bindValue(":invoice_id", invoice_id);
  
  if (!q.exec()) {
    db.rollback();
    return PrintInvoiceParams {};
  }
  
  while (q.next()) {
    PaymentData pdt;
    pdt.adminName = q.value("admin").toString();
    pdt.method = q.value("method").toString();
    pdt.amount = q.value("amount").toInt();
    pdt.paymentTime = q.value("payment_time").toDateTime();
    
    params.paymentList << pdt;
  }
  
  db.rollback();
  return params;
}

bool DatabaseInterface::saveInvoiceAndPayment(const InvoiceData& ida, int amount, const QString& method, QSqlRecord &ref) {
  auto db = QSqlDatabase::database("JUST-INV_DB", true);
  db.transaction();
  auto invr = createInvoice(ida, db);
  if(invr.isEmpty()) {
    return false;
  }
  
  if(!appendInvoiceItems(invr, ida.itemList, db)) {
    return false;
  }
  
  if (createPayment(invr, amount, method, db).isEmpty()) {
    return false;
  }
  ref = invr;
  emit tableUpdate( {"payments"} );
  return db.commit();
}

bool DatabaseInterface::savePayment(const QSqlRecord& invoiceRecord, int amount, const QString& method) {
  return !createPayment(invoiceRecord, amount, method, database()).isEmpty();
}

QSqlDatabase DatabaseInterface::database() const
{
  return QSqlDatabase::database("JUST-INV_DB", true);
}

bool DatabaseInterface::saveStoreInfoData(const StoreInfoData &d) {
  auto db = database();
  db.transaction();
  int affected = 0;
  QVariantList keys {"storeName", "storeAddr", "storePhone"};
  QVariantList values { d.storeName, d.storeAddr, d.storePhone};  
  QSqlQuery q(db);
  q.prepare(R"--(
    INSERT INTO store_data (key, val) VALUES (:config_key, :new_value)
        ON CONFLICT(key) DO UPDATE SET val = :new_value
        WHERE val != excluded.val;
      )--");
  q.bindValue(":config_key", keys);
  q.bindValue(":new_value", values);
  
  if( !q.execBatch() ) {
    qDebug() << "execBatch failed" << q.lastError().text();
    db.rollback();
    return false;
  }
  db.commit();
  return true;
}

InvoiceData DatabaseInterface::getInvoiceData(int invoice_id) const
{
  QSqlQuery q(database());
  q.prepare("SELECT * FROM invoices WHERE id = :iid");
  q.bindValue(":iid", invoice_id);
  if (q.exec() && q.next()) {
    InvoiceData invd;
    invd.adminName = q.value("admin").toString();
    invd.customerName = q.value("customer").toString();
    invd.customerPhone = q.value("customer_phone").toString();
    invd.dateString = q.value("invoice_date").toString();
    invd.total = q.value("total").toInt();
    invd.paid = q.value("paid").toInt();

    QSqlRecord invoiceRecord = q.record();
    q.prepare("SELECT * FROM invoice_item WHERE invoice_id = :iid");
    q.bindValue(":iid", invoice_id);
    if (q.exec()) {
      while (q.next()) {
        InvoiceData::ItemData a;
        a.productName = q.value("order_name").toString();
        a.unitPrice = q.value("price").toInt();
        a.unitQty = q.value("qty").toInt();
        a.subTotal = q.value("subTotal").toInt();
        invd.itemList << a;
      }
      return invd;
    }
  }
  return InvoiceData {};
}

bool DatabaseInterface::updateInvoice(int invoice_id, const InvoiceData &newData)
{
  auto db = database();
  db.transaction();
  QSqlRecord ir = createInvoice(newData, db);
  if (ir.isEmpty()) {
    db.rollback();
    return false;
  }
  if ( !appendInvoiceItems(ir, newData.itemList, db) ) {
    db.rollback();
    return false;
  }
  QSqlQuery q(db);
  q.prepare("UPDATE invoices SET (status, revision_ref) = ('revised', :old_id) WHERE id = :old_id");
  q.bindValue(":old_id", invoice_id);
  if ( !q.exec()) {
    db.rollback();
    return false;
  }
  if (q.numRowsAffected() == 0) {
    db.rollback();
    return false;
  }
  if (!db.commit()) {
    db.rollback();
    return false;
  }
  emit tableUpdate( {"invoices"} );
  return true;
}

