#include "customermodel.h"

CustomerModel::CustomerModel(QObject* parent)
  : QSqlTableModel(parent)
{
    setTable("customers");
    select();
    setHeaderData(0, Qt::Horizontal, "ID");
    setHeaderData(1, Qt::Horizontal, "Nama");
    setHeaderData(2, Qt::Horizontal, "Email");
    setHeaderData(3, Qt::Horizontal, "Telepon");
    setHeaderData(4, Qt::Horizontal, "Alamat");
    setHeaderData(5, Qt::Horizontal, "Admin Pembuat");
    setHeaderData(6, Qt::Horizontal, "Tgl");
}

CustomerModel::~CustomerModel(){}

QVariant CustomerModel::data(const QModelIndex& mi, int role) const
{
    if(role == Qt::TextAlignmentRole) {
        switch(mi.column()) {
            case 0:
            case 6:
            case 7:
                return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        }
    }
    return QSqlTableModel::data(mi, role);
}