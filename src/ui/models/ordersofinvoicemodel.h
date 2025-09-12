#ifndef ORDERSOFINVOICEMODEL_H
#define ORDERSOFINVOICEMODEL_H

#include "database.h"
#include <QSortFilterProxyModel>
#include <QLocale>

class OrdersOfInvoiceModel : public QSortFilterProxyModel {
  Q_OBJECT

public:
  OrdersOfInvoiceModel(int, Database *db, QObject *parent = nullptr);
  QVariant data(const QModelIndex &modelIndex, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
  Database *db;
  QLocale loc;
  QMap<int, QString> pmap;
};

#endif