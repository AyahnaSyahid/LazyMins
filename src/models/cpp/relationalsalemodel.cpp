#include "relationalsalemodel.h"
#include <QSqlTableModel>
#include <QSqlRelation>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QLocale>

#include <QtDebug>


RelationalSaleModel::RelationalSaleModel(QObject* parent)
  : QSqlRelationalTableModel(parent)
{
    setTable("sale");
    setRelation(1, QSqlRelation("customer", "id", "name"));
    setRelation(2, QSqlRelation("product", "id", "code"));
    select();
    setHeaderData(1, Qt::Horizontal, "Konsumen");
    setHeaderData(2, Qt::Horizontal, "Produk");
    setHeaderData(3, Qt::Horizontal, "Nama");
    setHeaderData(4, Qt::Horizontal, "Panjang");
    setHeaderData(5, Qt::Horizontal, "Lebar");
    setHeaderData(6, Qt::Horizontal, "QTY");
    setHeaderData(8, Qt::Horizontal, "Harga Jual");
    setHeaderData(9, Qt::Horizontal, "ID Pembayaran");
}

RelationalSaleModel::~RelationalSaleModel() {}

QVariant RelationalSaleModel::data(const QModelIndex& mi, int role) const
{
    if(!mi.isValid()) return QVariant();
    auto base = QSqlRelationalTableModel::data(mi, role);
    if(role == Qt::TextAlignmentRole)
    {
        switch(mi.column()) {
            case 2:
                return (int) Qt::AlignCenter;
            case 4:
            case 5:
            case 6:
            case 8:
            case 9:
                return (int) (Qt::AlignRight | Qt::AlignVCenter);
            default :
                return base;
        }
    } else if (role == Qt::DisplayRole) {
        switch(mi.column()) {
            case 4:
            case 5:
            case 6:
            case 8:
            case 9:
                return QLocale().toString(base.toDouble(), 'g', 9);
            default :
                return base;
        }
    } else if (role == Qt::ToolTipRole) {
        QString s("Tanggal : %1\nTotal Harga: Rp. %2");
        double h = index(mi.row(), 4).data(Qt::EditRole).toDouble() *
                   index(mi.row(), 5).data(Qt::EditRole).toDouble() *
                   index(mi.row(), 6).data(Qt::EditRole).toDouble() *
                   index(mi.row(), 8).data(Qt::EditRole).toDouble();
        return s.arg(QLocale().toString(mi.siblingAtColumn(10).data(Qt::EditRole).toDate(), "d MMM yyyy"), QLocale().toString(h, 'g', 9));
    }
    return base;
}

QVariant RelationalSaleModel::idOfIndex(const QModelIndex& mi) const {
    return mi.siblingAtColumn(0).data(Qt::EditRole);
}

QVariant RelationalSaleModel::customerId(const QModelIndex& mi) const {
    QSqlQuery q;
    q.prepare("SELECT customer_id FROM sale WHERE id = ?");
    q.addBindValue(idOfIndex(mi));
    if(q.exec() && q.next()){
        return q.value(0);
    }
    return QVariant();
}

bool RelationalSaleModel::isSameCustomer(const QModelIndexList& mil) const {
    if(mil.size() < 2) {
        return true;
    }
    auto zero = customerId(mil[0]);
    for(int i=1; i<mil.size(); ++i) {
        if(customerId(mil.at(i)) != zero)
            return false;
    }
    return true;
}

bool RelationalSaleModel::removeOrders(const QVariantList& ids) {
    QSqlQuery q("BEGIN TRANSACTION");
    for(auto id : ids) {
        q.prepare("DELETE FROM sale WHERE id = ?");
        q.addBindValue(id);
        if(!q.exec()) {
            qDebug() << q.lastError().text();
            q.exec("ROLLBACK");
            return false;
        }
    }
    auto commit = q.exec("COMMIT");
    if(!commit) {
        qDebug() << q.lastError().text();
        q.exec("ROLLBACK");
        return commit;
    }
    select();
    return commit;
}

QVariant RelationalSaleModel::paymentId(const QModelIndex& mi) const
{
    if(!mi.isValid()) return QVariant();
    return mi.siblingAtColumn(9).data(Qt::EditRole);
}

bool RelationalSaleModel::hasNoPayment(const QModelIndexList& mil) const
{
    for(auto mi : mil) {
        if(!paymentId(mi).isNull()) return false;
    }
    return true;
}