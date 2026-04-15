#include "helpers.h"
#include "src/models/ordermodel.h"
#include "src/managers/orderitemfinishingmanager.h"
#include "src/managers/orderitemmanager.h"
#include "src/managers/ordermanager.h"
#include "src/managers/invoicemanager.h"
#include "src/managers/stockmovementmanager.h"
#include "src/utils/sessionmanager.h"
#include "src/utils/sqltransaction.h"

#include <QSqlQuery>
#include <QSqlError>

namespace {
  int getAdminId () {
    auto &sm = SessionManager::instance();
    auto cu = sm.currentUser();
    return cu.has_value() ? (*cu).value("id").toInt() : 1;
  }
  
  qreal roundUpValue(double v, double prec = 100.0) {
    return qCeil(v * prec) / prec;
  }
}
const DBOperationHelper::OperationResult failResult { false, "" };

int DBOperationHelper::currentAdminId() { return getAdminId(); }

DBOperationHelper::OperationResult DBOperationHelper::createInstantOrder(
  const OrderHeader &header,
  const QList<OrderItem> &items, 
  const QString& invoiceCode,
  const QVariantMap& paymentInfo )
{ 
  SqlTransaction tr;
  if(!tr.started()) {
    return { false, "Gagal membuat transaksi Database :" + BaseManager::connection.lastError().text() };
  }
  
  int _admin_id = currentAdminId();
  OrderItemFinishingManager fm;
  OrderItemManager oim;
  OrderManager om;
  PaymentManager pym;
  ProductManager prm;
  StockMovementManager smm;
  
  // create order first
  auto opt_ord = om.create( {
    {"admin_id", _admin_id},
    {"order_number", header.order_number},
    {"customer_id", header.customer_id < 1 ? QVariant(QMetaType::fromType<int>()) : header.customer_id},
    {"customer_name", header.customer_name},
    {"discount_amount", header.discount_amount},
    {"price_level_id", header.price_level_id},
    {"priority", header.priority},
    {"notes", header.notes},
    {"internal_notes", header.internal_notes}
  } );
  
  if (!opt_ord) {
    return { false, "Gagal mendaftarkan order : " + om.errorString() };
  }
  
  int created_order_id = opt_ord->value("id").toInt();
  QString lastError = "";
  
  QList<int> createdItems;
  // Write Item to database
  for(auto const& order_item : items) {
    auto opt_oitem = oim.create({
      {"order_id", opt_ord->value("id")},
      {"product_id", order_item.product_id},
      {"product_name", order_item.product_name},
      {"sku", order_item.sku},
      {"quantity", order_item.quantity},
      {"unit", order_item.unit},
      {"size_width", order_item.size_width},
      {"size_height", order_item.size_height},
      {"use_area", order_item.use_area},
      {"sale_price", order_item.sale_price},
      {"base_price", order_item.base_price},
      {"discount_amount", order_item.discount_amount}
    });
    
    if(!opt_oitem) {
      lastError = oim.errorString();
      break ;
    }
    
    auto opt_prod = prm.getById(order_item.product_id);
    if (!opt_prod) {
      lastError = prm.errorString();
      break ;
    }
    
    qreal pr_stock = opt_prod->value("stock").toDouble();
    qreal c_qty = order_item.quantity;
    if (order_item.use_area) {
      c_qty = roundUpValue(c_qty * order_item.size_width * order_item.size_height);
    }
    
    auto opt_sm = smm.recordMovement( 
          order_item.product_id,
          "out",
          -c_qty,
          pr_stock,
          roundUpValue(c_qty - order_item.quantity),
          getAdminId(),
          "order_items",
          opt_oitem->value("id").toInt(),
          "Penjualan Produk (Instant Order)"
          );
    
    if (!opt_sm) {
      lastError = smm.errorString();
      break;
    }
    
    int current_item_id = opt_oitem->value("id").toInt();
    createdItems << current_item_id;
    // Write each finishings Item
    for(auto const& finishing_item : order_item.finishings) {
      auto opt_finitem = fm.create({
        {"order_item_id", current_item_id}, 
        {"finishing_id", finishing_item.finishing_id}, 
        {"finishing_name", finishing_item.finishing_name}, 
        {"quantity", finishing_item.quantity}, 
        {"finishing_price", finishing_item.finishing_price} 
      });
      
      if(!opt_finitem) {
        lastError = fm.errorString();
        break ;
      }
    }
    if (!lastError.isEmpty()) break;
  }

  if(!lastError.isEmpty()) {
    return { false, "Gagal menyimpan data" + lastError };
  }
  
  for(auto const& citem : createdItems) {
    if (!oim.recalculate(citem)) {
      return { false, oim.errorString() };
    }
  }
  
  // Register invoice and order
  QVariantMap inv_par ( {
    {"invoice_number", invoiceCode},
    {"customer_id", header.customer_id < 1 ? QVariant(QMetaType::fromType<qint64>()) : header.customer_id},
    {"customer_name", header.customer_name},
    {"customer_phone", header.customer_name},
    {"admin_id", getAdminId()},
    {"tax_amount", paymentInfo["tax_amount"]}
  });

  auto operation_2 = internalCreateInvoice(inv_par, createdItems);
  
  if (!operation_2.ok) {
    return operation_2;
  }
  
  int created_invoice_id = operation_2.data["invoice_id"].toInt();
  
  AkunTransaksiManager atm;
  auto opt_acc = atm.getByKode("CASH");
  if ( !opt_acc ) {
    return { false, QString("Tidak dapat menemukan akun_transaksi_id dengan kode '%1'").arg("CASH")};
  }
  
  auto opt_pay = pym.create({
    {"invoice_id", created_invoice_id},
    {"amount", paymentInfo["payment_amount"]},
    {"cash_received", paymentInfo["cash_received"]},
    {"cash_change", paymentInfo["cash_change"]},
    {"verification_status", "verified"},
    {"verified_by", getAdminId()},
    {"verified_at", QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss")},
    {"notes", "Pembayaran Instant Order"},
    {"admin_id", getAdminId()},
    {"akun_transaksi_id", opt_acc->value("id")}
  });
  
  if ( !opt_pay.has_value() ) {
    return { false, "Gagal membuat pembayaran : " + pym.errorString() };
  }
  
  if (!tr.commit()) {
    QString err = BaseManager::connection.lastError().text();
    return { false, "Tidak dapat melakukan commit : " + err };
  }

  tr.commit();
  return { true, "", {{"invoice_id", created_invoice_id}, {"payment_id", opt_pay->value("id")}} };
}

DBOperationHelper::OperationResult 
  DBOperationHelper::stockUpdate( const OrderItem& item, 
                                  const QString& tipe,
                                  const QString& notes,
                                  int adminId )
{
  return { false, "" };
}

DBOperationHelper::OperationResult DBOperationHelper::adjustProductStock( int product_id, qreal _final, const QString& notes) {
  SqlTransaction tr;
  if(!tr.started()) {
    return { false, "Gagal membuat transaksi Database :" + BaseManager::connection.lastError().text() };
  }
  
  ProductManager pm;
  StockMovementManager smm;
  
  auto opt_pro = pm.getById(product_id);
  if (!opt_pro) {
    return { false, "Tidak dapat menemukan data Produk" };
  }
  
  auto current_stock = opt_pro->value("stock").toDouble();
  qreal delta = roundUpValue(_final - current_stock);
  
  
  auto opt_sm = smm.recordMovement( 
        product_id, "adjustment", 
        delta, current_stock, _final,
        currentAdminId(), "", 0, notes);
  
  if (!opt_sm) {
    return { false, smm.errorString() };
  }

  tr.commit();
  return { true, "" };
}

DBOperationHelper::OperationResult DBOperationHelper::refillProductStock ( int productId, qreal stockIn, 
                                                                           const QString& supplier, 
                                                                           const QString& notes) {
  SqlTransaction tr;
  if(!tr.started()) {
    return { false, "Gagal membuat transaksi Database :" + BaseManager::connection.lastError().text() };
  }
  
  ProductManager pm;
  StockMovementManager smm;
  
  auto opt_pro = pm.getById(productId);
  if(!opt_pro) {
    return { false, "Tidak dapat menemukan data Produk" };
  }
  
  qreal current_stock = opt_pro->value("stock").toDouble();
  
  auto opt_sm = smm.recordMovement( 
    productId, "in", stockIn, current_stock,
    roundUpValue(stockIn + current_stock),
    currentAdminId(), "", 0, "Supplier : " + supplier + "\n" + notes);
  
  if (!opt_sm) {
    return { false, smm.errorString() };
  }
  
  tr.commit();
  return { true, "", {{"stock_movement_id", opt_sm->value("id")}} };
}

DBOperationHelper::OperationResult DBOperationHelper::internalCreateInvoice(const QVariantMap& param, QList<int> oids)
{
  QVariantMap paramCopy(param);
  InvoiceManager iman;
  paramCopy["admin_id"] = getAdminId();
  auto opt_inv = iman.create(paramCopy);
  if (!opt_inv.has_value()) {
    qDebug() << "internalCreateInvoice::Gagal" << iman.errorString();
    return { false, iman.errorString() };
  }
  
  if(!iman.addOrders(opt_inv->value("id").toInt(), oids)) {
    return { false, iman.errorString() };
  }
  
  if (!iman.recalculate(opt_inv->value("id").toInt())) {
    return { false, iman.errorString() };
  }

  return { true, "", {{"invoice_id", opt_inv->value("id")}}};
}

DBOperationHelper::OperationResult DBOperationHelper::createInvoiceForOrders(const QVariantMap& param, QList<int> oids)
{
  if (!BaseManager::connection.transaction())
    return { false, "Gagal membuat transaksi Database" };
  auto ires = internalCreateInvoice(param, oids);
  if (!ires.ok) {
    BaseManager::connection.rollback();
    return ires;
  }
  if (!BaseManager::connection.commit()) {
    BaseManager::connection.rollback();
    return { false, BaseManager::connection.lastError().text() };
  }
  return ires;
}

DBOperationHelper::OperationResult DBOperationHelper::createPaymentForOrders(const QVariantMap& inv, const QVariantMap& pay, QList<int> oids)
{
  if (!BaseManager::connection.transaction())
    return { false, "Gagal membuat transaksi Database" };
  auto ires = internalCreateInvoice(inv, oids);
  if (!ires.ok) {
    BaseManager::connection.rollback();
    return { false, BaseManager::connection.lastError().text() };
  }

  int invoice_id = ires.data["invoice_id"].toInt();
  
  PaymentManager payman;
  QVariantMap copyPay(pay);
  copyPay["invoice_id"] = invoice_id;
  copyPay["admin_id"] = currentAdminId();
  if (copyPay.contains("verified_at") && (!copyPay["verified_at"].isNull()))
    copyPay["verified_by"] = copyPay["admin_id"];
  
  auto opt_pay = payman.create(copyPay);
  if (!opt_pay) {
    BaseManager::connection.rollback();
    return { false, payman.errorString() };
  }
  
  if (!BaseManager::connection.commit()) {
    BaseManager::connection.rollback();
    return { false, BaseManager::connection.lastError().text() };
  }
  
  return { true, "", {{"invoice_id", invoice_id}, {"payment_id", opt_pay->value("id")}}};
}