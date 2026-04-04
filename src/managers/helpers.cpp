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
    {"internal_notes", "Order Instant"},
    {"admin_id", currentAdminId}
  };
  
  auto opt_invoice = invoiceManager.create(invoiceParam);
  
  if(!opt_invoice.has_value()) {
    con.rollback();
    auto err = invoiceManager.errorString();
    qWarning() << "createInstantOrderFailed"
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
    qWarning() << "createInstantOrderFailed"
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
      qWarning() << "createInstantOrderFailed on create orderItem"
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
        qWarning() << "createInstantOrderFailed on create finishingItem"
                   << err;
        return {false, err};
      }
      fi_total += fi.subtotal();
    }
    
    bool order_item_updated = orderItemManager.update(createdItemId, {{"finishing_total", fi_total}});
    if(!order_item_updated) {
      con.rollback();
      auto err = orderItemManager.errorString();
      qWarning() << "createInstantOrderFailed on create update finishing_total"
                 << err;
      return { false, err };
    }
  }

  PaymentManager paymentManager;
  auto pay_n = PaymentManager::generatePaymentNumber();
  QVariantMap paymentParams {
    {"payment_number", pay_n},
    {"invoice_id", createdInvoiceId},
    {"amount", paymentInfo.value("payment_amount", 0)},
    {"admin_id", currentAdminId},
    {"cash_received", paymentInfo.value("cash_received", 0)},
    {"cash_change", paymentInfo.value("cash_change", 0)}
  };
  
  auto opt_payment = paymentManager.create(paymentParams);
  
  if (!opt_payment.has_value()) {
    con.rollback();
    auto err = paymentManager.errorString();
    qWarning() << "createInstantOrderFailed on create create payment"
               << err;
    return { false, err };
  }
  
  auto createdPaymentId = (*opt_payment).value("id").toInt();
  
  TransaksiManager transaksiManager;
  
  auto opt_last_tr = transaksiManager.lastTransaction();
  
  int amount_before = 0,
      amount = paymentInfo.value("payment_amount").toInt(),
      amount_after = 0;
  
  if(opt_last_tr.has_value()) {
    amount_before = (*opt_last_tr).value("amount_after").toInt();
  }
  
  amount_after = amount_before + amount;
  
  auto tr_num = transaksiManager.generateTransactionNumber();
  
  QVariantMap transaksiParams {
    {"tr_num", tr_num},
    {"admin_id", currentAdminId},
    {"kategori_id", 1},
    {"tipe", "pemasukan"},
    {"deskripsi", "Pembayaran Cash"},
    {"amount_before", amount_before},
    {"amount", amount},
    {"amount_after", amount_after},
    {"payment_method", "cash"},
    {"reference_type", "payments"},
    {"reference_id", createdPaymentId}
  };
  
  auto opt_transaksi = transaksiManager.create(transaksiParams);
  
  if(!opt_transaksi.has_value()) {
    auto err = transaksiManager.errorString();
    qWarning() << "createInstantOrderFailed on logging Transaksi"
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
