#include "customeritem.h"
#include <QVariant>

CustomerItem::CustomerItem(qint64 i) :
    customer_id(i),
    DatabaseItem()
{
    q.prepare(R"-(
        SELECT customer_name, email, phone_number, address,
               created_by, created_at
          FROM customers
         WHERE customer_id = ?;
        )-");
    q.addBindValue(i);
    if(q.exec() && q.next()) {
        qValueAssign(customer_name, QString);
        qValueAssign(email, QString);
        qValueAssign(phone_number, QString);
        qValueAssign(address, QString);
        created_by = UserItem(q.value("created_by").toLongLong());
        qValueAssign(created_at, QDateTime);
    } else {
        debugError(q)
        customer_id = -1;
    }
}

CustomerItem::CustomerItem(const QString& n) :
    customer_name(n),
    DatabaseItem()
{
    q.prepare(R"-(
        SELECT customer_id, email, phone_number, address
               created_by, created_at
          FROM customers
         WHERE customer_name = ?;
        )-");
    q.addBindValue(n);
    if(q.exec() && q.next()) {
        qValueAssign(customer_id, qint64);
        qValueAssign(email, QString);
        qValueAssign(phone_number, QString);
        qValueAssign(address, QString);
        created_by = UserItem(q.value("created_by").toLongLong());
        qValueAssign(created_at, QDateTime);
    } else {
        debugError(q);
        customer_id = -1;
    }
}

bool CustomerItem::save() {
    if(created_by.user_id < 1) return false;
    if(customer_name.isEmpty()) return false;
    if(customer_id < 1) {
        // create mode
        // if(!email.isEmpty()) {
            // q.prepare("SELECT 1 FROM customers WHERE email = ?");
            // q.addBindValue(email);
            // if(q.exec() && q.next()) return false;
        // }
        q.prepare(R"-(
            INSERT INTO customers ( customer_name,
                                    email,
                                    phone_number,
                                    address,
                                    created_by,
                                    created_at )
                    VALUES (?, ?, ?, ?, ?, CURRENT_TIMESTAMP);
            )-");
        q.addBindValue(customer_name);
        q.addBindValue(email);
        q.addBindValue(phone_number);
        q.addBindValue(address);
        q.addBindValue(created_by.user_id);
        if(!q.exec()) {
            debugError(q)
            return false;
        }
        *this = CustomerItem(q.lastInsertId().toLongLong());
        return true;
    } else {
        // if(!email.isEmpty()) {
            // q.prepare("SELECT customer_id FROM customers WHERE email = ?");
            // q.addBindValue(email);
            // if(q.exec() && q.next()) {
                // if(q.value("customer_id").toLongLong() != customer_id) {
                    // return false;
                // }
            // }
        // }
        q.prepare(R"-(
            UPDATE customers SET ( customer_name, email, phone_number,
                                   address, created_by )
                    = (?, ?, ?, ?, ?) WHERE customer_id = ?;
            )-");
        q.addBindValue(customer_name);
        q.addBindValue(email);
        q.addBindValue(phone_number);
        q.addBindValue(address);
        q.addBindValue(created_by.user_id);
        q.addBindValue(customer_id);
        if(!q.exec()) {
            debugError(q)
            return false;
        }
        *this = CustomerItem(customer_id);
        return true;
    }
    return false;
}

bool CustomerItem::operator==(const DatabaseItem& d) const
{
    auto xx = dynamic_cast<const CustomerItem*>(&d);
    if(!xx) return false;
    return customer_id == xx->customer_id &&
           customer_name == xx->customer_name &&
           email == xx->email &&
           phone_number == xx->phone_number &&
           address == xx->address &&
           created_by == xx->created_by;
}

bool CustomerItem::erase()
{
    if(customer_id < 1) return true;
    q.prepare("DELETE FROM customers WHERE customer_id = ?");
    q.addBindValue(customer_id);
    return q.exec() && q.numRowsAffected();
}