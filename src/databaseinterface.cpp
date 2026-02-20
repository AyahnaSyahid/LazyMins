#include "databaseinterface.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <cmath>

namespace {
  std::optional<QSqlRecord> createInvoice(const InvoiceData& ida, QSqlDatabase &db) {
    QSqlQuery qi(db);
    qi.prepare( R"--( 
      INSERT INTO invoices ( invoice_date, admin, customer, customer_phone, total, created_by)
      VALUES (:invoice_date, :admin, :customer, :customer_phone, :total, :admin); )--");
    qi.bindValue(":invoice_date", ida.dateString);
    qi.bindValue(":admin", ida.adminName);
    qi.bindValue(":customer", ida.customerName);
    qi.bindValue(":customer_phone", ida.customerPhone.isEmpty() ? QVariant(QMetaType::fromType<QString>()): ida.customerPhone);
    qi.bindValue(":total", ida.total);
    if (!qi.exec()) {
      return std::nullopt;
    }
    int invId = qi.lastInsertId().toInt();
    qi.prepare("SELECT * FROM invoices WHERE id = :id");
    qi.bindValue(":id", invId);
    if(qi.exec() && qi.next()) {
      return qi.record();
    }
    return std::nullopt;
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
  
  std::optional<QSqlRecord> createPayment(const QSqlRecord& inv, const QString& by, int amount, const QString& method, QSqlDatabase  db) {
    QSqlQuery q(db);
    q.prepare(R"--(
      INSERT INTO payments (invoice_id, admin, amount, method, payment_time, received_by)
      VALUES (:invoice_id, :admin, :amount, :method, CURRENT_TIMESTAMP, :admin); 
      )--");
    q.bindValue(":invoice_id", inv.value("id"));
    q.bindValue(":admin", by);
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
    qDebug() << "create payment failed" << q.lastError().text();
    return std::nullopt;
  };

}; // namespace END

DatabaseInterface &DatabaseInterface::instance() {
  static DatabaseInterface di;
  return di;
};

bool DatabaseInterface::saveInvoiceData(const InvoiceData &ida){
  auto db = database();
  Transaction tr(db, __func__);
  auto createResult = createInvoice(ida, db);
  if (createResult) {
    if(appendInvoiceItems(createResult.value(), ida.itemList, db)) {
      tr.commit();
      emit saveDone(true);
      emit tableUpdate( {"invoices", "invoice_item"});
      return true;
    }
  }
  emit saveDone(false);
  return false;
}

std::optional<StoreInfoData> DatabaseInterface::getStoreInfo(QSqlDatabase &db) const {
  StoreInfoData storeInfo {};
  
  QSqlQuery q(db);
  if (!q.exec("SELECT key, val FROM store_data WHERE key IN ('storeName', 'storeAddr', 'storePhone')")) {
    qWarning() << "Query failed:" << q.lastError().text();
    return std::nullopt;
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

std::optional<PrintInvoiceParams> DatabaseInterface::getPrintInvoiceParams(int invoice_id) const {

  PrintInvoiceParams params {};
  auto db = database();
  Transaction tr(db, __func__);
  auto optStoreInfo = getStoreInfo(db);
  if (!optStoreInfo) {
    return std::nullopt;
  }
  
  params.storeInfo = optStoreInfo.value();
  
  QSqlQuery q(db);
  q.prepare("SELECT * FROM invoices WHERE id = :id");
  q.bindValue(":id", invoice_id);
  QSqlRecord invRec;
  if (q.exec() && q.next()) {
    invRec = q.record();
  } else {
    return std::nullopt;
  }
  
  params.invoiceCode = invRec.value("code").isNull() ? 
      QString("INV-%1-%2")
        .arg(invRec.value("invoice_date").toString().remove('-'))
        .arg(invRec.value("id").toString(), 4, QChar('0')) :
          invRec.value("code").toString();
  params.adminName = invRec.value("admin").toString();
  params.customerName = invRec.value("customer").toString();
  params.customerPhone = invRec.value("customer_phone").toString();
  params.invoiceDate = invRec.value("invoice_date").toString();
  
  q.prepare("SELECT * FROM invoice_item WHERE invoice_id = :invoice_id");
  q.bindValue(":invoice_id", invoice_id);
  
  if (!q.exec()) {
    return std::nullopt;
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
    return std::nullopt;
  }
  
  while (q.next()) {
    PaymentData pdt;
    pdt.adminName = q.value("admin").toString();
    pdt.method = q.value("method").toString();
    pdt.amount = q.value("amount").toInt();
    pdt.paymentTime = q.value("payment_time").toDateTime();
    params.paymentList << pdt;
  }
  return params;
}

bool DatabaseInterface::saveInvoiceAndPayment(const InvoiceData& ida, int amount, const QString& method, QSqlRecord &ref) {
  auto db = database();
  Transaction tr(db, __func__);
  
  auto invr = createInvoice(ida, db);
  if(!invr) {
    return false;
  }
  
  if(!appendInvoiceItems(*invr, ida.itemList, db)) {
    return false;
  }
  
  if (!createPayment(*invr, ida.adminName, amount, method, db)) {
    return false;
  }
  tr.commit();
  emit tableUpdate( {"payments"} );
  return true;
}

bool DatabaseInterface::savePayment(const QSqlRecord& invoiceRecord, const QString &by, int amount, const QString& method) {
  auto db = database();
  Transaction tr(db, __func__);
  
  auto optPayment = createPayment(invoiceRecord, by, amount, method, db);
  if ( !optPayment ) {
    return false;
  }
  tr.commit();
  emit tableUpdate ( { "invoices", "payments" });
  return true;
}

QSqlDatabase DatabaseInterface::database() const {
  return QSqlDatabase::database("JUST-INV_DB", true);
}

bool DatabaseInterface::saveStoreInfoData(const StoreInfoData &d) {
  auto db = database();
  Transaction tr(db, __func__);
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
    return false;
  }
  tr.commit();
  return true;
}

std::optional<InvoiceData> DatabaseInterface::getInvoiceData(int invoice_id) const {
  auto db = database();
  Transaction tr(db, __func__);
  QSqlQuery q(db);
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
      tr.commit();
      return invd;
    }
  }
  return std::nullopt;
}

std::optional<QSqlRecord> DatabaseInterface::getInvoiceRecord(int invoice_id) const {
  QSqlQuery q(database());
  q.prepare("SELECT * FROM invoices WHERE id = :id");
  q.bindValue(":id", invoice_id);
  if (q.exec() && q.next()) {
    return q.record();
  }
  return std::nullopt;
}

bool DatabaseInterface::updateInvoice(int invoice_id, const InvoiceData &newData, const QList<PaymentData> pd, int cashback) const {
  auto db = database();
  Transaction tr(db, __func__);
  // get in-database invoice data
  auto opt_invoiceData = getInvoiceData(invoice_id);
  if (opt_invoiceData) {
    InvoiceData oldInvoice = *opt_invoiceData;
    if (oldInvoice.paid == 0) {
      // this means invoice has not been issued
      // just remove old item_data and place new item_data, it wont hurt anyone
      QSqlQuery q(db);
      q.prepare("DELETE FROM invoice_item WHERE invoice_id = :iid");
      q.bindValue(":iid", invoice_id);
      if (q.exec()) {
        // all old invoice_item related to invoice_id has been deleted
        q.prepare("SELECT * FROM invoices WHERE id=:iid");
        q.bindValue(":iid", invoice_id);
        if (q.exec() && q.next()) {
          QSqlRecord invRec = q.record();
          if ( appendInvoiceItems(invRec, newData.itemList, db) ) {
            tr.commit();
            emit tableUpdate ( {"invoices", "invoice_item"} );
            return true;
          }
          qDebug() << "Update failed : tidak dapat menambahkan data";
          return false;
        }
        qDebug() << "Update failed : tidak dapat menemukan invoice record";
        return false;
      }
      qDebug() << "Update failed : tidak dapat menghapus invoice_item";
      return false;
    } else {
      // baik, sampai sini invoice ternyata telah memiliki data pembayaran
      // kita akan membuat invoice baru, kemudian mengubah status invoice lama juga menerapkan referensinya (superseeded)
      auto optNewInvoice = createInvoice(newData, db);
      if (optNewInvoice) {
        QSqlRecord newInvoice = *optNewInvoice;
        if (appendInvoiceItems(newInvoice, newData.itemList, db)) {
          int totalPaid = 0;
          if (pd.isEmpty()) {
            QSqlQuery movePayment(db);
            movePayment.prepare("UPDATE payments SET invoice_id = :newInvoiceId WHERE invoice_id = :oldInvoiceId AND status = 'active'");
            movePayment.bindValue(":newInvoiceId", newInvoice.value("id"));
            movePayment.bindValue(":oldInvoiceId", invoice_id);
            if (!movePayment.exec()) {
              qDebug() << "Update gagal : Pemindahan payment (invoice_id) gagal";
              return false;
            }
            QSqlQuery getPaidTotal(db);
            getPaidTotal.prepare("SELECT SUM(amount) FROM payments WHERE invoice_id = :newInvoiceId");
            getPaidTotal.bindValue(":newInvoiceId", newInvoice.value("id"));
            if (getPaidTotal.exec() && getPaidTotal.next()) {
              totalPaid = getPaidTotal.value(0).toInt();
            } else {
              qDebug() << "Update gagal : Pemindahan payment (invoice_id) gagal";
              return false;
            }
          } else {
            int paymentEnum = 0;
            for (const auto &p : pd) {
              auto optPaymentRecord = createPayment(newInvoice, p.adminName, p.amount, p.method, db);
              if (!optPaymentRecord) {
                qDebug() << "Update gagal : payment no " << paymentEnum ;
                return false;
              }
              totalPaid += p.amount;
              paymentEnum += 1;
            }
          }
          // apakah kita harus menerima parameter cashback ?
          // hanya terima cashback jika balance + cashback == 0
          int balance = newData.total - totalPaid;
          if (balance < 0) { // overpaid
            if (cashback == 0) {
              qDebug() << "Update gagal : diperlukan cashback untuk melanjutkan";
              return false;
            }
            if (balance + cashback != 0) {
              qDebug() << "Update gagal : jumlah cashback tidak terpenuhi";
              return false;
            }
            // add negative payment for cashback
            if (!createPayment(newInvoice, newData.adminName, -cashback, "CASH", db)) {
              qDebug() << "Update gagal : gagal mencatat cashback";
              return false;
            }
            // cashback added
          } else if (cashback != 0) {
            qDebug() << "Update gagal : unexpected cashback param";
            return false;
          }
          // sekarang kita buat referensi ke nota baru
          QSqlQuery ed(db);
          ed.prepare("UPDATE invoices SET status = 'revised', revision_ref = :newid WHERE id = :oldId");
          ed.bindValue(":newid", newInvoice.value("id"));
          ed.bindValue(":oldId", invoice_id);
          if (ed.exec()) {
            tr.commit();
            emit tableUpdate( {"invoices", "invoice_item", "payments"});
            return true;
          }
          qDebug() << "Update gagal : Gagal meng update status invoices lama";
          return false;
        }
        qDebug() << "Update gagal : Tidak dapat menambahkan data ke invoice baru";
        return false;
      }
      qDebug() << "Update gagal : Tidak dapat membuat invoice baru";
      return false;
    }
    qDebug() << "Error : Undefined Error";
    return false;
  }
  qDebug() << "invoice data not found";
  return false;
}

QList<QSqlRecord> DatabaseInterface::getPaymentRecords(int invoice_id) const {
  QSqlQuery q(database());
  q.prepare("SELECT * FROM payments WHERE invoice_id = :iid ");
  q.bindValue(":iid", invoice_id);
  if (q.exec()) {
    QList<QSqlRecord> recs;
    while (q.next()) {
      recs << q.record();
    }
    return recs;
  }
  return QList<QSqlRecord> {};
}

bool DatabaseInterface::recordCashFlow(const QString& adm, const QString& tipe, qlonglong amount, const QString& detail) const {
  QSqlQuery q(database());
  q.prepare("INSERT INTO catatan_keluar_masuk_cash (admin, tipe, amount, detail) "
            "VALUES (:adm, :tip, :amt, :det) ;");
  q.bindValue(":adm", adm);
  q.bindValue(":tip", tipe);
  q.bindValue(":amt", amount);
  q.bindValue(":det", detail);
  if (q.exec()) {
    emit tableUpdate({"catatan_keluar_masuk_cash"});
    return true;
  }
  return false;
}