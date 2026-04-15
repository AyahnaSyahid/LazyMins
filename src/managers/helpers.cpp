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
  SqlTransaction tr;
  if(!tr.started()) {
    return { false, "Gagal membuat transaksi Database :" + BaseManager::connection.lastError().text() };
  }
  
  InvoiceManager im;
  auto opt_inv = im.create({
    {"invoice_number", invoiceCode},
    {"customer_id", header.customer_id < 1 ? QVariant(QMetaType::fromType<qint64>()) : header.customer_id},
    {"customer_name", header.customer_name},
    {"customer_phone", header.customer_name},
    {"admin_id", getAdminId()},
    {"tax_amount", paymentInfo["tax_amount"]}
  });
  
  if (!opt_inv) {
    return { false, "Gagal membuat invoice : " + im.errorString() };
  }
  
  int created_invoice_id = opt_inv->value("invoice_id").toInt();
  
  OrderItemFinishingManager fm;
  OrderItemManager oim;
  OrderManager om;
  PaymentManager pym;
  ProductManager prm;
  StockMovementManager smm;
  
  // create order first
  auto opt_ord = om.create( {
    {"admin_id", getAdminId()},
    {"order_number", header.order_number},
    {"invoice_id", created_invoice_id},
    {"invoice_number", opt_inv->value("invoice_number")},
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
      c_qty = qCeil( (c_qty * order_item.size_width * order_item.size_height) * 100.0 ) / 100.0;
    }
    
    auto opt_sm = smm.recordMovement( 
          order_item.product_id,
          "out",
          -c_qty,
          pr_stock,
          qCeil((c_qty - order_item.quantity) * 100.0 ) / 100.0,
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
  
  if ( !om.recalculate(created_order_id) ) {
    return { false, om.errorString() };
  }
  
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
    {"notes", "Pembayaran Instant Order"},
    {"admin_id", getAdminId()},
    {"akun_transaksi_id", opt_acc->value("id")}
  });
  
  if ( !opt_pay.has_value() ) {
    return { false, "Gagal membuat pembayaran : " + pym.errorString() };
  }
  
  if ( !im.recalculate(created_invoice_id) ) {
    return { false, "Gagal memperbarui invoice : " + im.errorString() };
  }
  
  if (!tr.commit()) {
    QString err = BaseManager::connection.lastError().text();
    return { false, "Tidak dapat melakukan commit : " + err };
  }

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
  
  return { false, "" };
}

DBOperationHelper::OperationResult DBOperationHelper::refillProductStock ( int productId, qreal stockIn, const QString& supplier, const QString& notes) {
  
  return { false, "" };
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