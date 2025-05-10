#include "createordermodel.h"


#include <QSqlQueryModel>
#include <QSqlRelation>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QModelIndex>
#include <QLocale>

#include <QtMath>

#define IMPOSIBLE_INDEX -2000

QLocale loc(QLocale::Indonesian, QLocale::Indonesia);

QString _iquery( R"--(
    SELECT order_date AS Tanggal,
           products.name AS Produk,
           orders.name AS Nama,
           CASE use_area WHEN 1 THEN width ELSE '-' END AS Panjang,
           CASE use_area WHEN 1 THEN height ELSE '-' END AS Tinggi,
           quantity AS Qty,
           unit_price AS Harga,
           CEIL(width * height * quantity * unit_price/ 500) * 500 AS [Sub Total]
      FROM orders JOIN products USING (product_id, use_area) 
     WHERE status = 'OK' AND
           invoice_id IS NULL AND
           customer_id = %1
  ORDER BY Tanggal ASC;)--");

CreateOrderModel::CreateOrderModel(QObject* parent) :
relModel(new QSqlQueryModel(this)), QIdentityProxyModel(parent) {
    relModel->setQuery(_iquery.arg(IMPOSIBLE_INDEX));
    setSourceModel(relModel);
}

CreateOrderModel::~CreateOrderModel() {
    delete relModel;
}

QVariant CreateOrderModel::data(const QModelIndex& ix, int role) const {
    if(role == Qt::TextAlignmentRole) {
        switch(ix.column()) {
            case 0:
            case 1:
                return int(Qt::AlignCenter);
            case 3:
            case 4:
                return int(Qt::AlignRight | Qt::AlignVCenter);
            case 5:
            case 6:
            case 7:
                return int(Qt::AlignVCenter | Qt::AlignRight);
        }
    } else if(role == Qt::DisplayRole) {
        double val = sourceModel()->index(ix.row(), ix.column()).data(role).toDouble();
        switch(ix.column()) {
            case 5:
            case 6:
            case 7:
                return loc.toString(val, 'g', 16);
        }
    }
    return sourceModel()->data(ix, role);
}

void CreateOrderModel::setCustomerId(int _i) {
    relModel->setQuery(_iquery.arg(_i));
    emit reloaded();
}

void CreateOrderModel::reload() {
    relModel->setQuery(relModel->query().lastQuery());
    emit reloaded();
}