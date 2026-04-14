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
  return { false, "" };
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
  
  OrderManager orderManager;
  for(auto const& oid : oids) {
    if (!orderManager.update(oid, {{"invoice_id", opt_inv->value("id")}, {"invoice_number", opt_inv->value("invoice_number")}})) {
      qDebug() << "internalCreateInvoice::Gagal" << orderManager.errorString();
      return { false, orderManager.errorString()};
    }
  }
  return { true, "", {{"invoice_id", opt_inv->value("id").toInt()}}};
}

DBOperationHelper::OperationResult DBOperationHelper::createInvoiceForOrders(const QVariantMap& param, QList<int> oids)
{
  if (!BaseManager::connection.transaction())
    return { false, "Gagam membuat transaksi" };
  auto ires = internalCreateInvoice(param, oids);
  if (!ires.ok) {
    BaseManager::connection.rollback();
    return { false, BaseManager::connection.lastError().text() };
  }
  if (!BaseManager::connection.commit()) {
    BaseManager::connection.rollback();
    return { false, BaseManager::connection.lastError().text() };
  }
  return ires;
}

DBOperationHelper::OperationResult DBOperationHelper::createPaymentForOrders(const QVariantMap& inv, const QVariantMap& pay, QList<int> oids)
{
    return {false, ""};
}