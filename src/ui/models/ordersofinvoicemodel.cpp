#include "ordersofinvoicemodel.h"
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QDate>

OrdersOfInvoiceModel::OrdersOfInvoiceModel(int inv_id, Database *database, QObject *parent)
: db(database), pmap {}, QSortFilterProxyModel(parent) {
  auto omod = db->getTableModel("orders");
  if(omod) {
    setSourceModel(omod);
    auto rr = omod->record();
    int col_index = rr.indexOf("invoice_id");
    setFilterFixedString(QString::number(inv_id));
    setFilterKeyColumn(col_index);
  }
  auto pmod = db->getTableModel("products");
  for(int i=0; i<pmod->rowCount(); ++i) {
    auto pix = pmod->index(i, 0);
    pmap.insert(pix.data().toInt(), pix.siblingAtColumn(1).data().toString());
  }
}

QVariant OrdersOfInvoiceModel::data(const QModelIndex& modelIndex, int role) const {
  auto sm = sourceModel();
  if(modelIndex.isValid()) {
    auto si = mapToSource(modelIndex);
    if(role == Qt::DisplayRole) {
      switch (modelIndex.column()) {
        case 1:
          return loc.toString(si.data(role).toDate(), "dd/MM/yyyy");
        case 4:
          return pmap[si.data(role).toInt()];
        case 7:
          return loc.toString(si.data(role).toDouble() * si.siblingAtColumn(8).data(role).toDouble(), 'g', 3);
        case 11: {
          qreal width = si.siblingAtColumn(7).data().toDouble(),
                height = si.siblingAtColumn(8).data().toDouble();
          int qty = si.siblingAtColumn(9).data().toInt(),
              price = si.data().toInt();
          return loc.toString(width * height * qty * price, 'g', 13);
        }
        case 12:
          return loc.toString(si.data(role).toInt());
      }
    } else if (role == Qt::TextAlignmentRole) {
      switch (modelIndex.column()) {
        case 1:
          return Qt::AlignCenter;
        case 7:
        case 9:
        case 11:
        case 12:
          return (int) Qt::AlignRight | Qt::AlignVCenter;
      }
    }
  }
  return QSortFilterProxyModel::data(modelIndex, role);
}

QVariant OrdersOfInvoiceModel::headerData(int section, Qt::Orientation orient, int role) const {
  if(orient == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
      case 1:
        return "Tanggal";
      case 4:
        return "Produk";
      case 5:
        return "Nama";
      case 7:
        return "Ukuran";
      case 9:
        return "Qty";
      case 11:
        return "Total";
      case 12:
        return "Diskon";
    }
  }
  return QSortFilterProxyModel::headerData(section, orient, role);  
}