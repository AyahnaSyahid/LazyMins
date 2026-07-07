#include "instantorder.h"

#include "src/managers/financialledgerservice.h"
#include "src/managers/invoicemanager.h"
#include "src/managers/itemflowservice.h"
#include "src/managers/ordermanager.h"
#include "src/models/ordermodel.h"
#include "src/utils/sessionmanager.h"
#include "src/utils/sqltransaction.h"

bool InstantOrderController::create(const OrderHeader& oh, OrderModel* omodel,
                                    const QVariantMap& paymentAndInvoiceParam,
                                    QString* err) {
  if (!commitModel(omodel, err)) return false;

  FinancialLedgerService fls;
  PaymentManager pym;
  InvoiceManager invm;
  SqlTransaction tr;
  OrderManager om;
  if (!tr.started()) {
    if (err) {
      *err = "Tidak dapat memulai transaksi database";
    }
    return false;
  }

  QVariantMap paymentParam{
      {"amount", paymentAndInvoiceParam["payment_amount"]},
      {"cash_received", paymentAndInvoiceParam["cash_received"]},
      {"cash_change", paymentAndInvoiceParam["cash_change"]},
      {"akun_transaksi_id", 1},
      {"verification_status", "verified"},
      {"verified_by", SessionManager::instance().currentUserId()},
      {"verified_at",
       QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss")},
      {"admin_id", SessionManager::instance().currentUserId()}};

  QVariantMap invoiceParam{
      {"invoice_number", paymentAndInvoiceParam["invoice_number"]},
      {"customer_name", paymentAndInvoiceParam["name"]},
      {"customer_phone", paymentAndInvoiceParam["phone"]},
      {"price_level_id", paymentAndInvoiceParam["price_level"]},
      {"admin_id", SessionManager::instance().currentUserId()},
      {"tax_amount", paymentAndInvoiceParam["tax_amount"]},
      {"customer_id", oh.customer_id}};

  auto optInv = invm.create(invoiceParam);
  if (!optInv.has_value()) {
    if (err) *err = invm.errorString();
    return false;
  }

  if (!om.update(
          omodel->orderId(),
          {{"order_date",
            QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd hh:mm:ss")},
           {"deadline_date", oh.deadline_date.toString("yyyy-MM-dd hh:mm:ss")},
           {"completion_date",
            oh.completion_date.toString("yyyy-MM-dd hh:mm:ss")},
           {"discount_amount", oh.discount_amount},
           {"staging_status", "completed"}}))
    return false;
  
  invm.addOrders(optInv->value("id").toInt(), {omodel->orderId()});

  paymentParam["invoice_id"] = optInv->value("id").toInt();

  auto optPay = pym.create(paymentParam);
  if (!optPay.has_value()) {
    if (err) *err = pym.errorString();
    return false;
  }
  if (!fls.handlePayment(optPay->value("id").toInt())) {
    if (err) *err = fls.errorString();
    return false;
  }
  if (!tr.commit()) {
    if (err) *err = "Tidak dapat commit transaksi database";
    return false;
  }
  return true;
}

bool InstantOrderController::commitModel(OrderModel* omodel,
                                         QString* err) const {
  if (omodel->isDirty()) {
    if (!omodel->commit(BaseManager::connection)) {
      if (err) *err = "Tidak dapat OrderModel::commit Failed";
      return false;
    }
  }
  return true;
}
