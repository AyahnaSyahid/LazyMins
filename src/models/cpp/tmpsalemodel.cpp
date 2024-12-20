#include "tmpsalemodel.h"
#include "orderitem.h"
#include <QStandardItem>
#include <QStandardItemModel>
#include <QLocale>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QtMath>
#include <QtDebug>


TmpSaleModel::TmpSaleModel(QObject* parent)
    : QStandardItemModel(0, 13, parent)
{
    setHorizontalHeaderLabels( QStringList() << "ID" 
                                             << "Tanggal" 
                                             << "Name"
                                             << "W" 
                                             << "H" 
                                             << "Qty" 
                                             << "Harga" 
                                             << "Disc" 
                                             << "Total"
                                             << "Cost"
                                             << "P ID" // product_id 
                                             << "C ID" // customer_id
                                             << "INV"  // invoice_id
                                             );
}

TmpSaleModel::~TmpSaleModel() {}

QVariant TmpSaleModel::data(const QModelIndex& mi, int role) const {
    if(!mi.isValid()) return QVariant();
    if(role == Qt::TextAlignmentRole) {
        switch (mi.column()) {
            case 0:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
                return (int) (Qt::AlignRight | Qt::AlignVCenter);
            default:
                return QStandardItemModel::data(mi, role);
        }
    }
    if(role == Qt::DisplayRole) {
        auto def = QStandardItemModel::data(mi, Qt::EditRole);
        switch(mi.column()) {
            case 6:
            case 7:
            case 9:
                return QLocale().toString(def.toLongLong());
            case 1:
                return QLocale().toString(def.toDateTime().toLocalTime(), "yyyy-MM-dd");
            case 3:
            case 4:
            case 5:
                return QLocale().toString(def.toDouble(), 'g', 16);
            case 8:
                {
                    auto w = mi.siblingAtColumn(3).data(Qt::EditRole).toDouble();
                    auto h = mi.siblingAtColumn(4).data(Qt::EditRole).toDouble();
                    auto qty = mi.siblingAtColumn(5).data(Qt::EditRole).toDouble();
                    auto pr = mi.siblingAtColumn(6).data(Qt::EditRole).toLongLong();
                    auto ds = mi.siblingAtColumn(7).data(Qt::EditRole).toLongLong();
                    qint64 r = (qCeil(w * h * qty * pr / 100) * 100) - ds;
                    return QLocale().toString(r);
                }
            default:
                return QStandardItemModel::data(mi, role);
        }
    }
    return QStandardItemModel::data(mi, role);
}

Qt::ItemFlags TmpSaleModel::flags(const QModelIndex& ix) const {
    auto ret = QStandardItemModel::flags(ix);
    if(ix.column() == 8) {
        return ret ^ Qt::ItemIsEditable;
    }
    return ret;
}

bool TmpSaleModel::saveToSale()
{
    for(int i=0; i<rowCount(); ++i) {
        ;;
    }
    return false;
}