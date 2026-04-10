#include "helpers.h"
#include "src/utils/sessionmanager.h"
#include "src/managers/managers.h"
#include <QSqlQuery>
#include <QSqlError>

namespace {
  int getAdminId () {
    auto &sm = SessionManager::instance();
    auto cu = sm.currentUser();
    return cu.has_value() ? (*cu).value("id").toInt() : 1;
  }
  
  qreal roundedValue(double v) {
    return std::round(v * 10000.0) / 10000.0;
  }

}

int DBOperationHelper::currentAdminId() { return getAdminId(); }

DBOperationHelper::OperationResult DBOperationHelper::createInstantOrder(
  const OrderHeader &header,
  const QList<OrderItem> &items, 
  const QString& invoiceCode,
  const QVariantMap& paymentInfo )
{
  auto &con = BaseManager::connection;
  if (!con.transaction()) {
    return { false, "Tidak dapat melakukan transaksi database"};
  }
  
  InvoiceManager invoiceManager;
  int currentAdminId = getAdminId();
  // create invoice
  QVariantMap invoiceParam {
    {"invoice_number", invoiceCode},
    {"customer_id", header.customer_id},
    {"customer_name", header.customer_name},
    {"customer_phone", header.customer_phone},
    {"price_level_id", header.price_level_id},
    {"discount_amount", header.discount_amount},
    {"settlement_status", "paid"},
    {"staging_status", "sent"},
    {"tax_amount", paymentInfo.value("tax_amount", 0) },
    {"internal_notes", "Order Instant"},
    {"admin_id", currentAdminId}
  };

  auto opt_invoice = invoiceManager.create(invoiceParam);
  
  if(!opt_invoice.has_value()) {
    con.rollback();
    auto err = invoiceManager.errorString();
    qWarning() << "createInstantOrderFailed while createInvoice"
               << err;
    return {false, err};
  }
  
  int createdInvoiceId = (*opt_invoice).value("id").toInt();
  
  OrderManager orderManager;
  // Create Order
  
  QVariantMap orderParam {
    {"order_number", header.order_number},
    {"invoice_id", createdInvoiceId},
    {"invoice_number", invoiceCode},
    {"customer_id", header.customer_id},
    {"customer_name", header.customer_name},
    {"customer_phone", header.customer_phone},
    {"price_level_id", header.price_level_id},
    {"discount_amount", header.discount_amount},
    {"admin_id", currentAdminId}
  };
  
  auto opt_order = orderManager.create(orderParam);
  
  if(!opt_order.has_value()) {
    con.rollback();
    auto err = invoiceManager.errorString();
    qWarning() << "createInstantOrderFailed while createOrder"
               << err;
    return {false, err};
  }
  
  int createdOrderId = (*opt_order).value("id").toInt();
  
  OrderItemManager orderItemManager;
  OrderItemFinishingManager orderFinishingManager;
  ProductManager productManager;
  StockMovementManager stockManager;
  
  QVariantMap itemParams, finishingParams;
  for(auto const& item : items) {
    itemParams = {
      {"order_id", createdOrderId},
      {"product_id", item.product_id},
      {"product_name", item.product_name},
      {"sku", item.sku},
      {"quantity", item.quantity},
      {"unit", item.unit},
      {"use_area", item.use_area},
      {"size_width", item.use_area ? item.size_width : 1},
      {"size_height", item.use_area ? item.size_height : 1},
      {"sale_price", item.sale_price},
      {"base_price", item.base_price},
      {"discount_percentage", item.discount_percentage},
      {"discount_amount", item.discount_amount},
      {"notes", item.notes}
    };
    
    auto opt_orderItem = orderItemManager.create(itemParams);
    
    if(!opt_orderItem.has_value()) {
      con.rollback();
      auto err = orderItemManager.errorString();
      qWarning() << "createInstantOrderFailed while createOrderItem"
                 << err;
      return {false, err};
    }
    
    auto createdItemId = (*opt_orderItem).value("id").toInt();
    OrderItem temp(item);
    temp.id = createdItemId;
    temp.order_id = createdOrderId;
    
    auto stockUpdateResult = stockUpdate (temp, "out", "Penjualan Produk", currentAdminId);
    if (!stockUpdateResult.ok) {
      con.rollback();
      return stockUpdateResult;
    }
    
    int fi_total = 0;
    for(auto const& fi : item.finishings) {
      finishingParams = {
        {"order_item_id", createdItemId},
        {"finishing_id", fi.finishing_id},
        {"finishing_name", fi.finishing_name},
        {"quantity", fi.quantity},
        {"finishing_price", fi.finishing_price}
      };
      
      auto opt_finishingItem = orderFinishingManager.create(finishingParams);
      
      if(!opt_finishingItem.has_value()) {
        con.rollback();
        auto err = orderFinishingManager.errorString();
        qWarning() << "createInstantOrderFailed while createFinishingItem"
                   << err;
        return {false, err};
      }
      fi_total += fi.subtotal();
    }
    
    bool order_item_updated = orderItemManager.update(createdItemId, {{"finishing_total", fi_total}});
    if(!order_item_updated) {
      con.rollback();
      auto err = orderItemManager.errorString();
      qWarning() << "createInstantOrderFailed while create & update finishing_total"
                 << err;
      return { false, err };
    }
  }

  PaymentManager paymentManager;
  auto pay_n = PaymentManager::generatePaymentNumber();
  QVariantMap paymentParams {
    {"payment_number", pay_n},
    {"verification_status", "verified"},
    {"invoice_id", createdInvoiceId},
    {"amount", paymentInfo.value("payment_amount", 0)},
    {"admin_id", currentAdminId},
    {"akun_transaksi_id", 1}, // CASH
    {"cash_received", paymentInfo.value("cash_received", 0)},
    {"cash_change", paymentInfo.value("cash_change", 0)}
  };
  
  auto opt_payment = paymentManager.create(paymentParams);
  
  if (!opt_payment.has_value()) {
    con.rollback();
    auto err = paymentManager.errorString();
    qWarning() << "createInstantOrderFailed while createPayment"
               << err;
    return { false, err };
  }
  
  auto createdPaymentId = (*opt_payment).value("id").toInt();
  
  AkunTransaksiManager akunTransaksiManager;
  
  auto paymentAmount = paymentInfo.value("payment_amount").toInt();
  
  auto opt_accTr = akunTransaksiManager.getById(1);
  
  if (!opt_accTr.has_value()) {
    auto err = "CASH akun_transaksi tidak ditemukan";
    qWarning() << "createInstantOrderFailed while finding akun_transaksi by id"
               << err;
    return  { false, err };
  }
  
  auto saldoAkun = (*opt_accTr).value("saldo").toInt();
  
  if(!akunTransaksiManager.updateSaldo(1, paymentAmount)) {
    auto err = akunTransaksiManager.errorString();
    qWarning() << "createInstantOrderFailed while updating saldo akun_transaksi"
               << err;
    return  { false, err };
  }
  
  TransaksiManager transaksiManager;

  int amount_before = saldoAkun,
      amount = paymentAmount;
  
  
  auto tr_num = transaksiManager.generateTransactionNumber();
  
  QVariantMap transaksiParams {
    {"transaction_number", tr_num},
    {"admin_id", currentAdminId},
    {"kategori_id", 1},
    {"akun_id", 1},
    {"tipe", "pemasukan"},
    {"deskripsi", "Pembayaran Cash"},
    {"amount_before", amount_before},
    {"amount", amount},
    {"amount_after", amount_before + amount }, // dijumlahkan karena ini pembayaran
    {"payment_method", "cash"},
    {"reference_type", "payments"},
    {"reference_id", createdPaymentId}
  };
  
  auto opt_transaksi = transaksiManager.create(transaksiParams);
  
  if(!opt_transaksi.has_value()) {
    auto err = transaksiManager.errorString();
    qWarning() << "createInstantOrderFailed while logging Transaksi"
               << err;
    return { false, err };
  }
  
  if (!con.commit()) {
    qDebug() << "Error on COMMIT";
    auto error = con.lastError().text();
    con.rollback();
    return { false, error };
  }
  return { true, "" };
}

DBOperationHelper::OperationResult 
  DBOperationHelper::stockUpdate( const OrderItem& item, 
                                  const QString& tipe,
                                  const QString& notes,
                                  int adminId )
{
  ProductManager productManager;
  StockMovementManager stockManager;
  
  if (item.id < 1) {
    qWarning() << "stockUpdate Failed"
               << "order_items.id < 1";
    return  { false, "registering unsaved order_items" };
  }
  auto opt_product = productManager.getById(item.product_id);
    
  if(!opt_product.has_value()) {
    qWarning() << "stockUpdate Failed on getting product data";
    return {false, QString(" product data not found: %1").arg(item.product_id)};
  }
  
  auto rec_product = (*opt_product);
  auto sbefore = rec_product.value("stock").toDouble(),
       c_qty   = item.use_area ? static_cast<double>(item.quantity) * item.size_width * item.size_height : static_cast<double>(item.quantity);
  
  auto opt_stockMovement = 
        stockManager.recordMovement( item.product_id, tipe, 
                        c_qty, sbefore, tipe == "out" ? roundedValue(sbefore - c_qty) : roundedValue(sbefore + c_qty),
                        adminId, "orders", item.order_id, notes);
  
  auto product_stock_updated = 
        productManager.adjustStock(item.product_id, static_cast<double>(-c_qty));
  
  if (! (opt_stockMovement.has_value() && product_stock_updated) ) {
      qWarning() << "stockUpdate Failed";
      return { false, "Unable to update product stock"};
  }
  return { true, "" };
}

DBOperationHelper::OperationResult DBOperationHelper::adjustProductStock( int product_id, qreal _final, const QString& notes) {
  ProductManager productManager;
  StockMovementManager stockManager;
  
  BaseManager::connection.transaction();
  
  auto prd = *productManager.getById(product_id);
  auto currentData  = prd.value("stock").toDouble();
  auto delta = _final - currentData;
  
  bool pStockUpdated = productManager.update(product_id, {{"stock", _final}});
  if (!pStockUpdated) {
    qWarning() << "adjustProductStock Failed";
    BaseManager::connection.rollback();
    return { false, " Unable to update stock product"};
  }
  
  auto opt_movement =
      stockManager.recordMovement( product_id, "adjustment", 
                          qAbs(delta), currentData, roundedValue(currentData + delta),
                          getAdminId(), delta < 0 ? "Negative" : "Positive", -1, notes);
  if(!opt_movement.has_value()) {
    qWarning() << "adjustProductStock Failed" 
               << " Unable to log movement";
    auto err = stockManager.errorString();
    BaseManager::connection.rollback();
    return { false, err };
  }
  BaseManager::connection.commit();
  return { true, "" };
}

DBOperationHelper::OperationResult DBOperationHelper::refillProductStock ( int productId, qreal stockIn, const QString& supplier, const QString& notes) {
  ProductManager productManager;
  StockMovementManager stockManager;
  
  BaseManager::connection.transaction();
  
  auto prd = *productManager.getById(productId);
  auto sku = prd.value("sku").toString();
  auto unit = prd.value("unit").toString();
  auto c_stock = prd.value("stock").toDouble();
  bool productUpdateOk = productManager.update(productId, {{"stock", c_stock + stockIn }});
  auto opt_movement =
    stockManager.recordMovement( productId, "in", stockIn, c_stock, roundedValue(c_stock + stockIn), getAdminId(), "Pembelian Bahan", -1,
        QString("Pembelian %1 sebanyak %2 %3 dari %4 - %5").arg(sku, QLocale().toString(stockIn), unit, supplier, notes));
  if (productUpdateOk && opt_movement.has_value()) {
    BaseManager::connection.commit();
    return { true, "" };
  }
  OperationResult opr { false, "" };
  
  if (!productUpdateOk) opr.error = productManager.errorString();
  if (!opt_movement.has_value()) opr.error = stockManager.errorString();
  BaseManager::connection.rollback();
  return opr;
}

DBOperationHelper::OperationResult DBOperationHelper::createInvoiceForOrders(const QVariantMap& param, QList<int> oids)
{
    if (oids.isEmpty()) {
        return { false, "Daftar ID pesanan kosong." };
    }

    // Menggunakan QSqlDatabase secara eksplisit untuk manajemen transaksi yang lebih aman
    
    auto &db = BaseManager::connection;
    if (!db.transaction()) {
        return { false, "Gagal memulai transaksi database." };
    }

    InvoiceManager invoiceManager;
    
    QVariantMap copyParam(param);
    
    copyParam["admin_id"] = currentAdminId();

    // 1. Buat Invoice
    auto optInvoice = invoiceManager.create(copyParam);
    if (!optInvoice.has_value()) {
        QString err = invoiceManager.errorString();
        qWarning() << "DBOperationHelper::createInvoiceForOrders (Invoice Create) failed:" << err;
        db.rollback();
        return { false, err };
    }

    QSqlRecord recInv = *optInvoice;
    int invoiceId = recInv.value("id").toInt();
    QString invoiceNum = recInv.value("invoice_number").toString();

    // 2. Update Orders dalam satu batch
    // Mengonversi QList<int> ke QStringList untuk klausa IN
    QStringList idStrings;
    for (int id : oids) idStrings << QString::number(id);
    
    QSqlQuery q(db);
    QString queryStr = QString("UPDATE orders SET invoice_id = :iid, invoice_number = :inum "
                               "WHERE id IN (%1)").arg(idStrings.join(','));
    
    q.prepare(queryStr);
    q.bindValue(":iid", invoiceId);
    q.bindValue(":inum", invoiceNum);

    if (!q.exec()) {
        QString err = q.lastError().text();
        qWarning() << "DBOperationHelper::createInvoiceForOrders (Update Orders) failed:" << err;
        db.rollback();
        return { false, err };
    }

    // 3. Finalisasi Transaksi
    if (db.commit()) {
        return { true, "" };
    }

    QString lastErr = db.lastError().text();
    db.rollback();
    return { false, lastErr };
}

DBOperationHelper::OperationResult DBOperationHelper::createPaymentForOrders(const QVariantMap& inv, const QVariantMap& pay, QList<int> oids) {
  
  return { false, "Unimplemented"};
}