#include "pvordermodel.h"
#include <QLocale>
#include <QSqlError>
#include <QDateTime>
#include <QtMath>

#include <QtDebug>

PV_OrderModel::PV_OrderModel(QObject *parent)
: customer_id(-1), marked_id(), QSqlQueryModel(parent) {
    refreshModel();
}

void PV_OrderModel::refreshModel(){
    QString qry(R"-(
SELECT order_id AS ID,
       order_date AS Tanggal,
       order_name AS Nama,
       code AS Produk,
       CASE WHEN use_area THEN width_mm / 1000.0 ELSE NULL END AS W,
       CASE WHEN use_area THEN height_mm / 1000.0 ELSE NULL END AS H,
       quantity AS QTY,
       total_price AS Nilai
  FROM orders,
       products USING(product_id)
       WHERE invoice_id IS NULL
    )-");

    if(!marked_id.isEmpty()) {
        QStringList mid;
        for(auto b=marked_id.begin(); b!=marked_id.end(); ++b) {
            mid << QString::number(*b);
        }
        qry += QString(" AND order_id NOT IN (%1)").arg(mid.join(", "));
    }
    if(!customer_id < 1)
        qry += QString(" AND customer_id = %1 ").arg(customer_id);
    setQuery(qry);
    if(lastError().isValid()) {
        qDebug() << lastError().text();
    }
}

QVariant PV_OrderModel::data(const QModelIndex& mi, int role) const {
    auto def = QSqlQueryModel::data(mi, role);
    auto hn = headerData(mi.column(), Qt::Horizontal).toString();
    QLocale loc;
    if(role == Qt::DisplayRole) {
        if(hn == "Tanggal") {
            return loc.toString(def.toDateTime().toLocalTime(), "dddd, dd MMM yyyy");
        } else if(hn == "W" || hn == "H") {
            if(def.isNull()) {
                return "-";
            }
            return loc.toString(def.toDouble(), 'g', 6);
        } else if(hn == "Nilai") {
            return loc.toString(qCeil(def.toDouble() / 100) * 100);
        } else if(hn != "Produk") {
            return def.toString();
        }
    }
    if(role == Qt::TextAlignmentRole) {
        if(hn != "Tanggal" && hn != "Nama" && hn != "Produk") {
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        }
    }
    if(role == Qt::EditRole) {
        if(hn == "Nilai") {
            return qCeil(def.toDouble() / 100) * 100;
        }
    }
    return def;
}

void PV_OrderModel::setCustomer(qint64 n) {
    customer_id = n;
    refreshModel();
}
