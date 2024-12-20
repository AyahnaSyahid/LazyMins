#include "invoiceitem.h"

namespace GlobalNamespace {
    extern qint64 logged_user_id;
}

InvoiceItem::InvoiceItem(qint64 iid)
  :  invoice_id(iid), DatabaseItem()
{
    if(invoice_id > 0) {
        q.prepare("SELECT * FROM invoices WHERE invoice_id = ?");
        q.addBindValue(invoice_id);
        if(q.exec() && q.next()) {
            qValueAssign(user_id, qint64);
            qValueAssign(invoice_id, qint64);
            qValueAssign(customer_id, qint64);
            qValueAssign(modified_by, qint64);
            qValueAssign(total_amount, qint64);
            qValueAssign(total_paid, qint64);
            qValueAssign(discount_amount, qint64);
            qValueAssign(physical_iid, QString);
            qValueAssign(invoice_date, QDateTime);
            invoice_date = invoice_date.toLocalTime();
            qValueAssign(created_date, QDateTime);
            created_date = created_date.toLocalTime();
            qValueAssign(modified_date, QDateTime);
            modified_date = modified_date.toLocalTime();
            qValueAssign(due_date, QDateTime);
            due_date = due_date.toLocalTime();
            // getting orders
            q.prepare("SELECT order_id FROM orders WHERE invoice_id = ?");
            q.addBindValue(invoice_id);
            q.exec();
            while(q.next()) {
                _orders << q.value(0).toLongLong();
            }
        } else {
            invoice_id = -1;
        }
    }
}

InvoiceItem::~InvoiceItem(){}

QList<OrderItem> InvoiceItem::getOrders() const {
    QList<OrderItem> me;
    for(auto ii = _orders.cbegin(); ii != _orders.cend(); ++ii) {
        me << OrderItem(*ii);
    }
    return me;
}

bool InvoiceItem::removeOrder(qint64 oi) {
    OrderItem ord(oi);
    if(ord.order_id < 0) return false;
    ord.invoice_id = -1;
    auto iof = _orders.indexOf(oi);
    if(iof >= 0) {
        auto o = OrderItem(_orders.takeAt(iof));
        o.invoice_id = 0;
        return o.save();   
    }
    return false;
}
    
bool InvoiceItem::addOrder(qint64 oi) {
    // order harus sudah di simpan terlebih dahulu
    OrderItem order(oi);
    if(order.order_id < 1) return false;
    _orders << oi;
    return true;
}

bool InvoiceItem::save() {
    auto luid = GlobalNamespace::logged_user_id;
    if( luid < 1) return false;
    if(customer_id < 1) return false;
    q.exec("BEGIN TRANSACTION");
    if(invoice_id < 1) {
        q.prepare(R"-(INSERT INTO invoices (
                        user_id, customer_id, physical_iid,
                        invoice_date, due_date, 
                        discount_amount, created_date )
                       VALUES (?, ?, ?, ?, ?, ?, ?);
            )-");
        q.addBindValue(GlobalNamespace::logged_user_id);
        q.addBindValue(customer_id);
        q.addBindValue(physical_iid.isEmpty() ? QVariant() : physical_iid);
        q.addBindValue(invoice_date.toUTC());
        q.addBindValue(due_date.isValid() ? due_date.toUTC() : QVariant());
        q.addBindValue(discount_amount);
        q.addBindValue(QDateTime::currentDateTimeUtc());
        if(q.exec()) {
            if(q.exec("COMMIT")) {
                invoice_id = q.lastInsertId().toLongLong();
                return true;
            }
        }
        debugError(q);
        q.exec("ROLLBACK");
        return false;
    } else {
        // update
        q.prepare(R"::(
            UPDATE invoices SET (
                customer_id, invoice_date, physical_iid, due_date, 
                discount_amount, modified_date, modified_by )
            = (?, ?, ?, ?, ?, ?, ?) WHERE invoice_id = ?;
        )::");
        q.addBindValue(customer_id);
        q.addBindValue(invoice_date.toUTC());
        q.addBindValue(physical_iid.isEmpty() ? QVariant() : physical_iid);
        q.addBindValue(invoice_date.addDays(2).toUTC());
        q.addBindValue(discount_amount);
        q.addBindValue(modified_date.toUTC());
        q.addBindValue(luid);
        q.addBindValue(invoice_id);
        if(q.exec()){
            if(q.exec("COMMIT")){
                return true;
            }
        }
        debugError(q);
        q.exec("ROLLBACK");
        return false;
    }
    return false;
}

bool InvoiceItem::erase() {
    q.prepare("DELETE FROM invoices WHERE invoice_id = ?");
    q.addBindValue(invoice_id);
    return q.exec();
}

bool InvoiceItem::addPayment(InvoicePaymentsItem* ipi) {
    if(!ipi) return false;
    ipi->modified_date = QDateTime::currentDateTimeUtc();
    ipi->modified_by = GlobalNamespace::logged_user_id;
    return ipi->save();
}

void InvoiceItem::updateTotal() {
    if(invoice_id < 0) return;
    qint64 ta = 0;
    q.exec("BEGIN TRANSACTION");
    q.prepare("SELECT SUM(total_price) FROM orders WHERE invoice_id = ?");
    q.addBindValue(invoice_id);
    if(q.exec() && q.next()) {
        ta = q.value(0).toLongLong();
    } else {
        q.exec("ROLLBACK");
        return;
    }
    if(ta != total_amount) {
        q.prepare("UPDATE invoices SET(total_amount) = (?) WHERE invoice_id = ?");
        q.addBindValue(ta);
        q.addBindValue(invoice_id);
        if (q.exec()) {
            if (q.exec("COMMIT")) {
                total_amount = ta;
                return;
            }
        }
    }
    q.exec("ROLLBACK");
}

void InvoiceItem::updatePaid() {
    if(invoice_id < 0) return;
    qint64 tp = 0;
    q.exec("BEGIN TRANSACTION");
    q.prepare(R"-(
SELECT sum(p.amount) OVER (PARTITION BY p.invoice_id) AS Paid
  FROM invoices i
       JOIN invoice_payments p USING ( invoice_id ) WHERE invoice_id = ?
       )-");
   q.addBindValue(invoice_id);
   if(!(q.exec() && q.next())) {
       q.exec("ROLLBACK");
       return ;
   }
   tp = q.value(0).toLongLong();
   if(tp != total_paid) {
       q.prepare("UPDATE invoices SET(total_paid) = ? WHERE invoice_id = ?");
       q.addBindValue(tp);
       q.addBindValue(invoice_id);
       if(q.exec() && q.exec("COMMIT")) {
           total_paid = tp;
           return;
       }
   }
   q.exec("ROLLBACK");
}