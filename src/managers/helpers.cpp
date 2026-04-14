#include "helpers.h"
#include "src/models/ordermodel.h"
#include "src/managers/orderitemfinishingmanager.h"
#include "src/managers/orderitemmanager.h"
#include "src/managers/ordermanager.h"
#include "src/managers/invoicemanager.h"
#include "src/utils/sessionmanager.h"

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
  if(!BaseManager::connection.transaction()) {
    return { false, "Gagal membuat transaksi Database :" + BaseManager::connection.lastError().text() };
  }
  
  OrderItemFinishingManager fm;
  OrderItemManager oim;
  OrderManager om;
  
  // create order first
  QList<int> createdOrderIds;
  auto opt_ord = om.create( {
    {"admin_id", getAdminId()},
    {"order_number", oh.order_number},
    {"customer_id", oh.customer_id < 1 ? QVariant(QMetaType::from<int>()) : oh.customer_id},
    {"customer_name", oh.customer_name},
    {"discount_amount", oh.discount_amount},
    {"price_level_id", oh.price_level_id},
    {"priority", oh.priority},
    {"notes", oh.notes},
    {"internal_notes", oh.internal_notes}
  } );
  
  if (!opt_ord) {
    return { false, "Gagal mendaftarkan order :" om.errorString() };
  }
  
  createdOrderIds = opt_ord->value("id").toInt()

  QString lastError = "";
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

    for(auto const& finishing_item : order_item.finishings) {
      auto opt_finitem = fm.create({
        {"order_item_id", finishing_item.order_item_id}, 
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