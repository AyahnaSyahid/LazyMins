#include "invoicepaymentsitem.h"
#include "invoiceitem.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QtDebug>
InvoicePaymentsItem::InvoicePaymentsItem(qint64 ip)
  :  payment_id(-1), DatabaseItem()
{
    q.prepare("SELECT * FROM invoice_payments WHERE payment_id = ?");
    q.addBindValue(payment_id);
    if(q.exec() && q.next()) {
        qValueAssign(payment_date, QDateTime);
        payment_date = payment_date.toLocalTime();
        qValueAssign(invoice_id, qint64);
        qValueAssign(amount, qint64);
        qValueAssign(method, QString);
        qValueAssign(note, QString);
        qValueAssign(created_by, qint64);
        qValueAssign(created_date, QDateTime);
        created_date = created_date.toLocalTime();
        qValueAssign(modified_by, qint64);
        qValueAssign(modified_date, QDateTime);
        modified_date = modified_date.toLocalTime();
    } else {
        payment_id = -1;
    }
}

InvoicePaymentsItem::~InvoicePaymentsItem(){}

bool InvoicePaymentsItem::save()
{
    if(invoice_id < 0) {
        return false;
    }
    if(payment_id < 0) {
        // new
        q.prepare("INSERT INTO invoice_payments (payment_date, invoice_id, amount, "
                  "method, created_by, created_date ) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
        q.addBindValue(payment_date.toUTC());
        q.addBindValue(invoice_id);
        q.addBindValue(amount);
        q.addBindValue(method);
        q.addBindValue(created_by);
        q.addBindValue(created_date.toUTC());
        if(!q.exec()){
            if(q.lastError().isValid()) {
                qDebug() << __FILE__ << __LINE__;
                qDebug() << q.lastError().text();
            } else {
                qDebug() << __FILE__ << __LINE__;
                qDebug() << "Error tidak diketahui saat menyimpan data payments";
            }
        } else {
            payment_id = q.lastInsertId().toLongLong();
            if (invoice_id > 0) {
                InvoiceItem ii(invoice_id);
                ii.updatePaid();
            }
            return true;
        }
    } else {
        q.prepare("UPDATE invoice_payments SET ("
                "invoice_id, amount, payment_date, "
                "created_date, created_by, method, "
                "modified_by, modified_date) "
                  "= (?, ?, ?, ?, ?, ?, ?, ?) WHERE payment_id = ?");
        q.addBindValue(invoice_id < 0 ? QVariant() : invoice_id);
        q.addBindValue(amount);
        q.addBindValue(payment_date.toUTC());
        q.addBindValue(created_date.toUTC());
        q.addBindValue(created_by);
        q.addBindValue(method);
        q.addBindValue(modified_by);
        q.addBindValue(modified_date.isValid() ? modified_date.toUTC() : QVariant());
        q.addBindValue(payment_id);
        if(!q.exec()){
            if(q.lastError().isValid()) {
                qDebug() << __FILE__ << __LINE__;
                qDebug() << q.lastError().text();
            } else {
                qDebug() << "Error tidak diketahui saat menyimpan data payments";
            }
        } else {
            payment_id = q.lastInsertId().toLongLong();
            if (invoice_id > 0) {
                InvoiceItem ii(invoice_id);
                ii.updatePaid();
            }
            return true;
        }
    }
    return false;
}

bool InvoicePaymentsItem::erase()
{
    if(!q.exec(
        QString("DELETE FROM invoice_payments WHERE payment_id = %1")
        .arg(payment_id))) {
            debugError(q);
            return false;
    }
    if (invoice_id > 0) {
        InvoiceItem ii(invoice_id);
        ii.updatePaid();
    }
    return true;
}