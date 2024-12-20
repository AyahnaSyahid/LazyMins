#include "salemodel.h"
#include <QLocale>

SaleModel::SaleModel(QObject *parent)
  : QSqlTableModel(parent)
{
    setTable("sale");
    select();
}

SaleModel::~SaleModel() {}

QVariant SaleModel::data(const QModelIndex& mi, int role) const
{
    if(!mi.isValid()) return QVariant();
    auto va = QSqlTableModel::data(mi, role);
    if(role == Qt::DisplayRole) {
        switch(mi.column()) {
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
                return QLocale().toString(va.toDouble(), 'g', 9);
            default :
                return QSqlTableModel::data(mi, role);
        }
    } else if(role == Qt::TextAlignmentRole) {
        switch(mi.column()) {
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
                return (int) (Qt::AlignRight | Qt::AlignVCenter);
            default :
                return QSqlTableModel::data(mi, role);
        }
    }
    return QSqlTableModel::data(mi, role);
}