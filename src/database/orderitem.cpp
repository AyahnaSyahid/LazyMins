#include "orderitem.h"
#include "invoiceitem.h"
#include "customeritem.h"

namespace GlobalNamespace {
    extern qint64 logged_user_id;
}

OrderItem::OrderItem(qint64 oid) : 
    order_id(oid), 
    DatabaseItem()
{
    q.prepare("SELECT * FROM orders WHERE order_id = ?");
    q.addBindValue(oid);
    if(q.exec() && q.next()) {
        qValueAssign(order_id, qint64)
        qValueAssign(order_name, QString)
        qValueAssign(product_id, qint64)
        qValueAssign(customer_id, qint64)
        if(!q.value("invoice_id").isNull())
            qValueAssign(invoice_id, qint64)
        else
            invoice_id = -1;
        qValueAssign(quantity, qint64)
        qValueAssign(cost_production, qint64)
        qValueAssign(selling_price, qint64)
        qValueAssign(discount_amount, qint64)
        // qValueAssign(discount_percentage, qreal)
        qValueAssign(width_mm, qint64)
        qValueAssign(height_mm, qint64)
        qValueAssign(order_date, QDateTime)
        qValueAssign(total_price, qint64)
        order_date = order_date.toLocalTime();
    } else {
        order_id = -1;
    }
}

OrderItem::~OrderItem(){}

bool OrderItem::save() {
    if(order_id < 1) {
        // create
        q.prepare(R"-(
            INSERT INTO orders (
                order_name, product_id, invoice_id, order_date, 
                customer_id, quantity, cost_production,
                selling_price, discount_amount, width_mm,
                height_mm, created_by)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
            )-");
        q.addBindValue(order_name);
        q.addBindValue(product_id);
        q.addBindValue(invoice_id < 1 ? QVariant() : invoice_id);
        q.addBindValue(order_date.toUTC());
        q.addBindValue(customer_id);
        q.addBindValue(quantity);
        q.addBindValue(cost_production);
        q.addBindValue(selling_price);
        q.addBindValue(discount_amount);
        // q.addBindValue(discount_percentage == 0 ? QVariant() : discount_percentage);
        q.addBindValue(width_mm == 0 ? QVariant() : width_mm);
        q.addBindValue(height_mm == 0 ? QVariant() : height_mm);
        q.addBindValue(GlobalNamespace::logged_user_id);
        if(!q.exec()) {
            debugError(q);
            return false;
        }
        *this = OrderItem(q.lastInsertId().toLongLong());
        if (invoice_id > 0) {
            InvoiceItem ii(invoice_id);
            ii.updateTotal();
            ii.updatePaid();
        }
    } else {
        q.prepare(R"-(
            UPDATE orders SET (
                order_name, product_id, invoice_id, order_date, 
                customer_id, quantity, cost_production,
                selling_price, discount_amount, width_mm,
                height_mm, modified_date, modified_by)
            = (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) WHERE order_id = ?;
            )-");
        
        q.addBindValue(order_name);
        q.addBindValue(product_id);
        q.addBindValue(invoice_id < 1 ? QVariant() : invoice_id);
        q.addBindValue(order_date.toUTC());
        q.addBindValue(customer_id);
        q.addBindValue(quantity);
        q.addBindValue(cost_production);
        q.addBindValue(selling_price);
        q.addBindValue(discount_amount);
        // q.addBindValue(discount_percentage == 0 ? QVariant() : discount_percentage);
        q.addBindValue(width_mm == 0 ? QVariant() : width_mm);
        q.addBindValue(height_mm == 0 ? QVariant() : height_mm);
        q.addBindValue(QDateTime::currentDateTimeUtc());
        q.addBindValue(GlobalNamespace::logged_user_id);
        q.addBindValue(order_id);
        if(!q.exec()) {
            debugError(q);
            qDebug() << q.executedQuery();
            return false;
        }
        *this = OrderItem(order_id);
        if (invoice_id > 0) {
            InvoiceItem ii(invoice_id);
            ii.updateTotal();
            ii.updatePaid();
        }
    }
    return true;
}

bool OrderItem::erase() {
    q.prepare("DELETE FROM orders WHERE order_id = ?");
    q.addBindValue(order_id);
    if(!q.exec()) {
        debugError(q);
        return false;
    }
    if (invoice_id > 0) {
            InvoiceItem ii(invoice_id);
            ii.updateTotal();
            ii.updatePaid();
        }
    return true;
}