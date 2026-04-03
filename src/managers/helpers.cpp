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
}

DBOperationHelper::CreateInstantOrderResult DBOperationHelper::createInstantOrder(
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
  // create invoice
  QVariantMap invoiceParam {
    {"invoice_number", invoiceCode},
    {"customer_id", header.customer_id},
    {"customer_name", header.customer_name},
    {"customer_phone", header.customer_phone},
    {"price_level_id", header.price_level_id},
    {"discount_amount", header.discount_amount},
    {"internal_notes", "Order Instant"},
    {"admin_id", getAdminId()}
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
    {"invoice_id", invoice_id},
    {"invoice_number", invoiceCode},
    {"customer_id", header.customer_id},
    {"customer_name", header.customer_name},
    {"customer_phone", header.customer_phone},
    {"price_level_id", header.price_level_id},
    {"discount_amount", header.discount_amount},
    {"admin_id", getAdminId()}
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
      {"order_id", order_id},
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
    auto opt_product = productManager.getById(item.product_id);
    
    if(!opt_product.has_value()) {
      con.rollback();
      qWarning() << "createInstantOrderFailed on getting product data";
      return {false, "product data not found"};
    }
    
    auto opt_stockMovement = 
          stockManager.recordMovement( item.product_id, 
                                       "out", );
    
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
  
  auto pay_n = PaymentManager::generatePaymentNumber();
  qp.bindValue(":payment_number", pay_n);
  qp.bindValue(":invoice_id", invoice_id);
  qp.bindValue(":payment_amount", paymentInfo.value("payment_amount", 0));
  qp.bindValue(":admin_id", getAdminId());
  qp.bindValue(":cash_received", paymentInfo.value("cash_received", 0));
  qp.bindValue(":cash_change", paymentInfo.value("cash_change", 0));
  
  if (!qp.exec()) {
    con.rollback();
    qDebug() << "Error on creating payment";
    return { false, qp.lastError().text() };
  }
  
  auto createdPaymentId = qp.lastInsertId().toInt();
  
  qp.exec("SELECT COALESCE(amount_after, 0) as AFT FROM transaksi ORDER BY id DESC LIMIT 1");
  qp.next();
  
  auto amount_before = qp.value("AFT").toInt(), 
       amount        = paymentInfo.value("payment_amount").toInt();
  
  auto amount_after = amount_before + amount;
  
  qp.prepare(R"-(
    INSERT INTO transaksi (
      transaction_number, admin_id, kategori_id, tipe, deskripsi, amount_before, amount, amount_after, payment_method, reference_type, reference_id)
    VALUES (
      :tr_num, :admin_id, :kategori_id, :tipe, :deskripsi, :amount_before, :amount, :amount_after, :payment_method, :reference_type, :reference_id)
  )-");
  
  TransaksiManager trm;
  auto tr_num = trm.generateTransactionNumber();
  qp.bindValue(":tr_num", tr_num);
  qp.bindValue(":admin_id", getAdminId());
  qp.bindValue(":kategori_id", 1);
  qp.bindValue(":tipe", "pemasukan");
  qp.bindValue(":deskripsi", "Pembayaran Cash");
  qp.bindValue(":amount_before", amount_before);
  qp.bindValue(":amount", amount);
  qp.bindValue(":amount_after", amount_after);
  qp.bindValue(":payment_method", "cash");
  qp.bindValue(":reference_type", "payments");
  qp.bindValue(":reference_id", createdPaymentId);
  
  if(!qp.exec()) {
    qDebug() << "Error : Gagal mencatat transaksi";
    qDebug() << qp.lastError().text();
    con.rollback();
    return { false, qp.lastError().text() };
  }
  
  if (!con.commit()) {
    qDebug() << "Error on COMMIT";
    auto error = con.lastError().text();
    con.rollback();
    return { false, error };
  }
  return { true, "" };
}