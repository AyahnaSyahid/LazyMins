#include "produkmodel.h"
#include "helper.h"
#include "productitem.h"
#include <QSqlQuery>

ProdukModel::ProdukModel(QObject* parent)
  : QSqlTableModel(parent)
{
    setTable("products");
    select();
    setHeaderData(0, Qt::Horizontal, "ID");
    setHeaderData(1, Qt::Horizontal, "Kode");
    setHeaderData(2, Qt::Horizontal, "Nama Default");
    setHeaderData(3, Qt::Horizontal, "QR/BARCODE");
    setHeaderData(4, Qt::Horizontal, "PCost");
    setHeaderData(5, Qt::Horizontal, "PSale");
    setHeaderData(6, Qt::Horizontal, "UK Area");
    setHeaderData(7, Qt::Horizontal, "Deskripsi");
    setHeaderData(8, Qt::Horizontal, "Dibuat");
    setHeaderData(9, Qt::Horizontal, "Diubah");
}

ProdukModel::~ProdukModel(){}

QVariant ProdukModel::data(const QModelIndex& mi, int role) const
{
    return QSqlTableModel::data(mi, role);
}

bool ProdukModel::addProduct(const QString& kode, 
                             const QString& name, 
                             const QString& qr,
                             const qreal cost ,
                             const qreal price,
                             const qreal area,
                             const QString& desc)
{
    auto cts = StringHelper::currentDateTimeString();
    ProductItem pi(kode);
    if(pi.product_id > 0) return false;
    
    pi.name = name;
    pi.qr_bar = qr;
    pi.cost = cost;
    pi.price = price;
    pi.use_area = area;
    pi.description = desc;
    if(pi.save()) {
        emit produkBaru(pi.product_id);
        select();
        return true;
    }
    return false;
}