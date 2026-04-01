#include "helpers.h"
#include "src/utils/sessionmanager.h"
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
  
  // create invoice
  QSqlQuery q(con);
  q.prepare(R"--(
    INSERT INTO invoices ( invoice_number, 
      customer_id, customer_name, customer_phone, 
      price_level_id, discount_amount, tax_amount, due_date,
      status, issue_date, internal_notes, admin_id ) 
    VALUES ( :invoice_number, 
      :customer_id, :customer_name, :customer_phone, 
      :price_level_id, :discount_amount, 0, date('now'),
      'paid', date('now'), :internal_notes, :admin_id);
  )--");
  q.bindValue(":invoice_number", invoiceCode);
  q.bindValue(":customer_id", header.customer_id);
  q.bindValue(":customer_name", header.customer_name);
  q.bindValue(":customer_phone", header.customer_phone);
  q.bindValue(":price_level_id", header.price_level_id);
  q.bindValue(":discount_amount", header.discount_amount);
  q.bindValue(":internal_notes", "Order Instant");
  q.bindValue(":admin_id", getAdminId());
  
  if(!q.exec()) {
    con.rollback();
    qDebug() << "Error on Create Invoices";
    return {false, q.lastError().text()};
  }
  
  int invoice_id = q.lastInsertId().toInt();
  
  q.prepare(R"--(
    INSERT INTO orders ( order_number, invoice_id, invoice_number,
      customer_id, customer_name, customer_phone,
      price_level_id, discount_amount, tax_amount,
      status, priority, order_date, deadline_date, completion_date,
      payment_status, paid_amount, internal_notes, admin_id )
    VALUES (:order_number, :invoice_id, :invoice_number,
      :customer_id, :customer_name, :customer_phone,
      :price_level_id, :discount_amount, 0,
      'completed', 'instant', date('now'), date('now'), date('now'),
      'paid', 0, 'Order Instant', :admin_id )
  )--");
  
  q.bindValue(":order_number", header.order_number);
  q.bindValue(":invoice_id", invoice_id);
  q.bindValue(":invoice_number", invoiceCode);
  q.bindValue(":customer_id", header.customer_id);
  q.bindValue(":customer_name", header.customer_name);
  q.bindValue(":customer_phone", header.customer_phone);
  q.bindValue(":price_level_id", header.price_level_id);
  q.bindValue(":discount_amount", header.discount_amount);
  q.bindValue(":admin_id", getAdminId());
  
  if(!q.exec()) {
    con.rollback();
    qDebug() << "Error on Create orders";
    return {false, q.lastError().text()};
  }
  
  int order_id = q.lastInsertId().toInt();
  QString prepareQueryItem(R"-(
    INSERT INTO order_items ( order_id, 
      product_id, product_name,    sku, 
      quantity,   unit,            use_area,            size_width,          size_height,
      sale_price, base_price,      discount_percentage, discount_amount,     notes )
    VALUES ( :order_id, 
      :product_id, :product_name, :sku, 
      :quantity,   :unit,         :use_area,            :size_width,         :size_height,
      :sale_price, :base_price,   :discount_percentage, :discount_amount, :notes )
  )-");
  
  QString prepareQueryItemFinishing(R"-(
    INSERT INTO order_item_finishings ( order_item_id,
      finishing_id, finishing_name, 
      quantity, finishing_price )
    VALUES ( :order_item_id,
      :finishing_id, :finishing_name, 
      :quantity, :finishing_price )
  )-");
  
  QSqlQuery qf(con), qi(con);
  
  for(auto const& item : items) {
    qf.prepare(prepareQueryItem);
    qf.bindValue(":order_id", order_id);
    qf.bindValue(":product_id", item.product_id);
    qf.bindValue(":product_name", item.product_name);
    qf.bindValue(":sku", item.sku);
    qf.bindValue(":quantity", item.quantity);
    qf.bindValue(":unit", item.unit);
    qf.bindValue(":use_area", item.use_area);
    qf.bindValue(":size_width", item.use_area ? item.size_width : 1);
    qf.bindValue(":size_height", item.use_area ? item.size_height : 1);
    qf.bindValue(":sale_price", item.sale_price);
    qf.bindValue(":base_price", item.base_price);
    qf.bindValue(":discount_percentage", item.discount_percentage);
    qf.bindValue(":discount_amount", item.discount_amount);
    qf.bindValue(":notes", item.notes);
    
    if(!qf.exec()) {
      con.rollback();
      qDebug() << "Error on Create order_items";
      return {false, qf.lastError().text()};
    }
    
    auto item_id = qf.lastInsertId().toInt();
    
    int fi_total = 0;
    for(auto const& fi : item.finishings) {
      qi.prepare(prepareQueryItemFinishing);
      qi.bindValue(":order_item_id", item_id);
      qi.bindValue(":finishing_id", fi.finishing_id);
      qi.bindValue(":finishing_name", fi.finishing_name);
      qi.bindValue(":quantity", fi.quantity);
      qi.bindValue(":finishing_price", fi.finishing_price);
      if(!qi.exec()) {
        con.rollback();
        qDebug() << "Error on Create order_item_finishings";
        return { false, qi.lastError().text() };
      }
      fi_total += fi.subtotal();
    }
    
    qf.prepare("UPDATE order_items SET finishing_total = :finishing_total WHERE id = :id");
    qf.bindValue(":finishing_total", fi_total);
    qf.bindValue(":id", item_id);

    if(!qf.exec()) {
      con.rollback();
      qDebug() << "Error on updating finishing_total";
      return { false, qf.lastError().text() };
    }
  }
  
  QSqlQuery qp(con);
  qp.prepare(R"-(
    INSERT INTO payments (
      payment_number, invoice_id,
      amount, 
      cash_received, cash_change,
      payment_status, admin_id, verified_by, verified_at, payment_date )
    VALUES ( :payment_number, :invoice_id,
             :payment_amount,
             :cash_received, :cash_change,
             'verified', :admin_id, :admin_id, datetime('now'), datetime('now'))
  )-");
  
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