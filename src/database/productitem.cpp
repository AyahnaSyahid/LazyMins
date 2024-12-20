#include "productitem.h"
#include <QVariant>

ProductItem::ProductItem(qint64 id) :
  product_id(id), DatabaseItem()
{
    q.prepare(R"-(
        SELECT * FROM products WHERE product_id = ?;
    )-");
    q.addBindValue(product_id);
    if(q.exec() && q.next()) {
        qValueAssign(code, QString);
        qValueAssign(name, QString);
        qValueAssign(qr_bar, QString);
        qValueAssign(cost, qint64);
        qValueAssign(price, qint64);
        qValueAssign(use_area, bool);
        qValueAssign(description, QString);
        qValueAssign(created_at, QDateTime);
        qValueAssign(updated_at, QDateTime);
    } else {
        debugError(q)
        product_id = -1;
    }
};

ProductItem::ProductItem(const QString& n) :
  code(n), product_id(-1), DatabaseItem()
{
    q.prepare(R"-(
        SELECT * FROM products WHERE code = ? ;
        )-");
    q.addBindValue(n);
    if(q.exec() && q.next()) {
        qValueAssign(product_id, qint64);
        qValueAssign(name, QString);
        // qValueAssign(code, QString);
        qValueAssign(qr_bar, QString);
        qValueAssign(cost, qint64);
        qValueAssign(price, qint64);
        qValueAssign(use_area, bool);
        qValueAssign(description, QString);
        qValueAssign(created_at, QDateTime);
        qValueAssign(updated_at, QDateTime);
    }
    debugError(q)
}

bool ProductItem::save()
{
    if(product_id < 1) {
        // Create New
        if(code.isEmpty()) return false;
        if(ProductItem(code).product_id > 0) return false;
        
        q.prepare("INSERT INTO products"
                        "( code, name, qr_bar, cost, price, use_area, description, created_at, updated_at)"
                 " VALUES (?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP);");
        q.addBindValue(code);
        q.addBindValue(name);
        q.addBindValue(qr_bar.isEmpty() ? QVariant() : qr_bar);
        q.addBindValue(cost);
        q.addBindValue(price);
        q.addBindValue(use_area);
        q.addBindValue(description);
        if(q.exec()){
            *this = ProductItem(q.lastInsertId().toLongLong());
            return true;
        }
        debugError(q);
    } else {
        if(*this == ProductItem(product_id)) return true;
        q.prepare("UPDATE products SET (code, name, qr_bar, cost, price, use_area, description, updated_at ) "
                    "= (?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP) WHERE product_id = ?");
        q.addBindValue(code);
        q.addBindValue(name);
        q.addBindValue(qr_bar.isEmpty() ? QVariant() : qr_bar);
        q.addBindValue(cost);
        q.addBindValue(price);
        q.addBindValue(use_area);
        q.addBindValue(description);
        q.addBindValue(product_id);
        if(q.exec()){
            *this = ProductItem(product_id);
            return true;
        }
        debugError(q)
    }
    return false;
}

bool ProductItem::erase()
{
    if(product_id < 1) return true;
    q.prepare("DELETE FROM products WHERE product_id = ?");
    q.addBindValue(product_id);
    return q.exec();
}

bool ProductItem::operator==(const DatabaseItem& di) const {
    auto pi = dynamic_cast<const ProductItem*>(&di);
    if(!pi) return false;
    return product_id == pi->product_id &&
           code == pi->code &&
           name == pi->name &&
           qr_bar == pi->qr_bar &&
           cost == pi->cost &&
           price == pi->price &&
           use_area == pi->use_area &&
           description == pi->description;
}